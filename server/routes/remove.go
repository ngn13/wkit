package routes

import (
	"net/http"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_remove(c *fiber.Ctx) error {
	var (
		client *database.Client
		db     *database.Type
		err    error
	)

	client = c.Locals("client").(*database.Client)
	db = c.Locals("database").(*database.Type)

	if err = db.ClientDel(client.ID); err != nil {
		log.Fail("failed to remove client %s from database: %s", client.ID, err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	if err = db.Save(); err != nil {
		log.Fail("failed to save the database: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	return util.Redirect(c, "/")
}
