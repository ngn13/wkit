#include "../inc/cmds.h"

#include <string.h>
#include <unistd.h>
#include <errno.h>

/*

 * chdir command, well, changes directory to a provided directory
 * fails if, well, if chdir fails

*/

char *cmd_chdir(job_t *job) {
  cmd_recv_all(job);

  // chdir to the provided dir
  if (chdir(job->data) != 0)
    return strerror(errno);

  return "success";
}
