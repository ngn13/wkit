package routes

import (
	"fmt"
	"net/http"
	"path"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/util"
)

func isPathSafe(c *fiber.Ctx, dir string, page string) bool {
	conf := c.Locals("config").(*config.Type)
	return page == "login" ||
		path.Clean(dir) == fmt.Sprintf("%s/%s", conf.Path, "static")
}

func VerifyAuth(c *fiber.Ctx) error {
	var (
		db    *database.Type
		token string
		dir   string
		page  string
	)

	db = c.Locals("database").(*database.Type)
	token = c.Cookies("token")

	if token != "" && db.TokenExists(token) < 0 {
		token = ""
	}

	dir, page = path.Split(c.Path())

	if token != "" && page == "login" {
		return util.Redirect(c, "/")
	}

	if token == "" && !isPathSafe(c, dir, page) {
		return util.Redirect(c, "/login")
	}

	c.Locals("token", token)
	return c.Next()
}

func GET_login(c *fiber.Ctx) error {
	return util.Render(c, "login")
}

func POST_login(c *fiber.Ctx) error {
	var (
		db   *database.Type
		conf *config.Type

		token string
		err   error
	)

	db = c.Locals("database").(*database.Type)
	conf = c.Locals("config").(*config.Type)

	body := struct {
		Password string `form:"password"`
	}{}

	if err = c.BodyParser(&body); err != nil {
		return util.RenderErr(c, "invalid form data", http.StatusBadRequest)
	}

	if body.Password != conf.Password {
		return util.Render(c, "login", fiber.Map{
			"error": "invalid password",
		})
	}

	token = db.TokenAdd()

	c.Cookie(&fiber.Cookie{
		Name:  "token",
		Value: token,
	})

	return util.Redirect(c, "/")
}

func GET_logout(c *fiber.Ctx) error {
	db := c.Locals("database").(*database.Type)
	token := c.Locals("token").(string)

	db.TokenDel(token)

	return util.Redirect(c, "/")
}
