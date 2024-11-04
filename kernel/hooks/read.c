#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_read = NULL;

asmlinkage int64_t h_read(const struct pt_regs *r) {
  if (_read == NULL)
    _read = hooks_find_orig(__NR_read);

  return _read(r);
}
