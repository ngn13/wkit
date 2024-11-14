#include "../inc/cmds.h"
#include <stdlib.h>

/*

 * first it deletes the kernel module, the save file and itself
 * then it unloads the kernel module
 * lastly sets "should_quit" to break the main loop 

*/

// see main.c
extern bool should_quit;

char *cmd_destruct(job_t *job) {
  cmd_recv_all(job);

  // TODO: delete self, the kernel module, the save file and unload the kernel module

  should_quit = true;
  return "success";
}
