package routes

import (
	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/util"
)

func GET_new(c *fiber.Ctx) error {
	return util.Render(c, "new")
}

func POST_new(c *fiber.Ctx) error {

}
