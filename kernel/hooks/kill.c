#include "../inc/hook.h"
#include "../inc/util.h"

syscall_t *_kill = NULL;

asmlinkage int64_t h_kill(const struct pt_regs *r) {
  orig_get(_kill, "kill");
  orig_call(_kill);
}
