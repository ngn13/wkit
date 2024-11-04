#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_write = NULL;

asmlinkage int64_t h_write(const struct pt_regs *r) {
  if (_write == NULL)
    _write = hooks_find_orig(__NR_write);

  return _write(r);
}
