#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_chdir = NULL;

asmlinkage int64_t h_chdir(const struct pt_regs *r) {
  return 0;

  if (_chdir == NULL)
    _chdir = hooks_find_orig(__NR_chdir);

  return _chdir(r);
}
