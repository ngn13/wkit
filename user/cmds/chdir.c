#include "../inc/cmds.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

/*

 * chdir command, well, changes directory to a provided directory
 * fails if, well, if chdir fails

*/

// see main.c
extern bool should_jitter;

char *cmd_chdir(job_t *job) {
  cmd_recv_all(job);

  // chdir to the provided dir
  if (chdir(job->data) != 0)
    return strerror(errno);

  // don't jitter after chdir
  should_jitter = false;

  return "success";
}
