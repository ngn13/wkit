#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_statx = NULL;

asmlinkage int64_t h_statx(const struct pt_regs *r) {
  if (_statx == NULL)
    _statx = hooks_find_orig(__NR_statx);

  return _statx(r);
}
