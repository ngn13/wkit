package joblist

import (
	"io"

	"github.com/ngn13/shrk/server/util"
)

var JOB_ID_SIZE = 6

type Type struct {
	Jobs  []Job
	Count int
}

func New() (*Type, error) {
	var jobs Type
	return &jobs, nil
}

func (jl *Type) Add(client_id string, cmd byte, data io.ReadSeeker, _res ...io.WriteSeeker) (*Job, error) {
	var (
		res io.WriteSeeker
		job Job
	)

	if res = nil; len(_res) > 0 {
		res = _res[0]
	}

	job = Job{
		ClientID: client_id,
		PacketID: 0,
		ID:       util.RandomStr(JOB_ID_SIZE),
		Command:  cmd,
		Data:     data,
		Result:   res,
		Done:     false,
		DoneChan: make(chan bool),
	}

	jl.Jobs = append(jl.Jobs, job)
	jl.Count++

	return &job, nil
}

func (jl *Type) Get(job_id string) (*Job, int) {
	for i := range jl.Jobs {
		if jl.Jobs[i].ID == job_id {
			return &jl.Jobs[i], i
		}
	}
	return nil, -1
}

func (jl *Type) Del(job_id string) {
	_, indx := jl.Get(job_id)
	jl.Jobs = append(jl.Jobs[:indx], jl.Jobs[indx+1:]...)
}

func (jl *Type) Find(client_id string) (*Job, int) {
	for i := range jl.Jobs {
		if jl.Jobs[i].ClientID == client_id {
			return &jl.Jobs[i], i
		}
	}
	return nil, -1
}
