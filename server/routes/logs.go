package routes

import (
	"bytes"
	"io"
	"net/http"
	"os"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_logs(c *fiber.Ctx) error {
	var (
		lf      *os.File
		content []byte
		err     error
	)

	if lf, err = os.Open(log.FILE); err != nil {
		log.Fail("failed to open the log file: %s", err.Error())
		return util.RenderErr(c, "failed to open the log file", http.StatusInternalServerError)
	}

	if content, err = io.ReadAll(lf); err != nil {
		log.Fail("failed to read the log file: %s", err.Error())
		return util.RenderErr(c, "failed to read the log file", http.StatusInternalServerError)
	}

	content = bytes.ReplaceAll(content, []byte(log.COLOR_BLUE), nil)
	content = bytes.ReplaceAll(content, []byte(log.COLOR_YELLOW), nil)
	content = bytes.ReplaceAll(content, []byte(log.COLOR_CYAN), nil)
	content = bytes.ReplaceAll(content, []byte(log.COLOR_RED), nil)
	content = bytes.ReplaceAll(content, []byte(log.COLOR_RESET), nil)

	return util.Render(c, "logs", fiber.Map{
		"content": string(content),
	})
}
