#include "../inc/hook.h"
#include <linux/module.h>

syscall_t *_open   = NULL;
syscall_t *_openat = NULL;

asmlinkage int64_t h_open(const struct pt_regs *r) {
  if (_open == NULL)
    _open = hooks_find_orig(__NR_open);

  return _open(r);
}

asmlinkage int64_t h_openat(const struct pt_regs *r) {
  if (_openat == NULL)
    _openat = hooks_find_orig(__NR_openat);

  return _openat(r);
}
