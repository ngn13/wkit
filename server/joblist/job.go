package joblist

import (
	"io"
	"time"
)

const (
	CMD_SHELL byte = 'S'
	CMD_CHDIR byte = 'C'
	CMD_LIST  byte = 'L'
	CMD_INFO  byte = 'I'

	timeout time.Duration = 60 * time.Second
)

type Job struct {
	ID       string         // job ID
	ClientID string         // C2 client ID
	PacketID uint64         // last received packet ID for this job
	Command  byte           // command
	Data     io.ReadSeeker  // command data
	Result   io.WriteSeeker // command result
	Done     bool           // is job completed?
	DoneChan chan bool
}

func (j *Job) Wait() bool {
	if j.Done {
		return true
	}

	select {
	case <-j.DoneChan:
		return true
	case <-time.After(timeout):
		return false
	}
}
