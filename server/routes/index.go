package routes

import (
	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/util"
)

func isActive(c *database.Client) bool {
	return c.IsActive()
}

func GET_index(c *fiber.Ctx) error {
	data := c.Locals("database").(*database.Type)
	clients := []*database.Client{}

	for i := range data.Clients {
		clients = append(clients, &data.Clients[i])
	}

	return util.Render(c, "index", fiber.Map{
		"clients": clients,
	})
}
