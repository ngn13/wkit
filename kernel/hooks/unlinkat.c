#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_unlinkat = NULL;

asmlinkage int64_t h_unlinkat(const struct pt_regs *r) {
  if (_unlinkat == NULL)
    _unlinkat = hooks_find_orig(__NR_unlinkat);

  return _unlinkat(r);
}
