package util

import (
	"net/http"
	"net/url"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/config"
)

func Qescape(s string) string {
	return url.QueryEscape(s)
}

func Render(c *fiber.Ctx, p string, m ...fiber.Map) error {
	var (
		conf *config.Type
		fmap fiber.Map
	)

	conf = c.Locals("config").(*config.Type)

	if len(m) >= 1 && m[0] != nil {
		fmap = m[0]
	} else {
		fmap = fiber.Map{}
	}

	// stuff we need all the time
	fmap["version"] = conf.Version
	fmap["qescape"] = Qescape
	fmap["path"] = conf.Path

	return c.Render(p, fmap)
}

func RenderErr(c *fiber.Ctx, err string, code int) error {
	c.Status(code)
	return Render(c, "error", fiber.Map{
		"error": err,
	})
}

func RenderNginxNotFound(c *fiber.Ctx) error {
	c.Status(http.StatusNotFound)
	return Render(c, "notfound")
}
