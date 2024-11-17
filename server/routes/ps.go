package routes

import (
	"fmt"
	"net/http"
	"sort"
	"strconv"
	"strings"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

type proc struct {
	PID     int64
	PPID    int64
	UID     int64
	GID     int64
	VmSize  uint64
	Cmdline string
}

func (p *proc) UsedMem() string {
	// kb to bytes with *1024
	return util.HumanizeSize(p.VmSize * 1024)
}

func newProc(s string) (*proc, error) {
	var (
		p     proc
		sects []string
		err   error
	)

	if !strings.Contains(s, "\\") {
		return nil, fmt.Errorf("%s", s)
	}

	if sects = strings.Split(s, "\\"); len(sects) < 6 {
		return nil, fmt.Errorf("process doesn't contain enough sections")
	}

	if p.PID, err = strconv.ParseInt(sects[0], 10, 32); err != nil {
		return nil, fmt.Errorf("failed to parse the process PID: %s", err.Error())
	}

	if p.PPID, err = strconv.ParseInt(sects[1], 10, 32); err != nil {
		return nil, fmt.Errorf("failed to parse the proccess PPID: %s", err.Error())
	}

	if p.UID, err = strconv.ParseInt(sects[2], 10, 32); err != nil {
		return nil, fmt.Errorf("failed to parse the process UID: %s", err.Error())
	}

	if p.GID, err = strconv.ParseInt(sects[3], 10, 64); err != nil {
		return nil, fmt.Errorf("failed to parse the process GID: %s", err.Error())
	}

	if p.VmSize, err = strconv.ParseUint(sects[4], 10, 64); err != nil {
		return nil, fmt.Errorf("failed to parse the process vmsize: %s", err.Error())
	}

	for i := 5; i < len(sects); i++ {
		p.Cmdline += sects[i]
	}

	if p.Cmdline == "" {
		return nil, fmt.Errorf("failed to parse the process cmdline")
	}

	return &p, nil
}

func GET_ps(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type

		res_str   string
		procs_str []string

		processes []*proc
		job       *joblist.Job
		proc      *proc

		err error
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	data := util.NewBuffer("-")
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_PS, data, res); err != nil {
		log.Fail("failed to add process list job: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("process list job timed out for %s", client.ID)
		return util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	res_str = res.String()
	procs_str = strings.Split(res_str, "\\\\")

	for _, p := range procs_str {
		if p == "" {
			continue
		}

		if proc, err = newProc(p); err != nil {
			log.Debg("failed to load the process for %s: %s", client.ID, err.Error())
			return util.RenderErr(c,
				fmt.Sprintf("process list job failed: %s", err.Error()), http.StatusGatewayTimeout,
			)
		}

		processes = append(processes, proc)
	}

	sort.Slice(processes, func(i, j int) bool {
		return processes[i].PID < processes[j].PID
	})

	return util.Render(c, "ps", fiber.Map{
		"client":    client,
		"processes": processes,
	})
}
