#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_newfstatat = NULL;

asmlinkage int64_t h_newfstatat(const struct pt_regs *r) {
  if (_newfstatat == NULL)
    _newfstatat = hooks_find_orig(__NR_newfstatat);

  return _newfstatat(r);
}
