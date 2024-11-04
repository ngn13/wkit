#include "../inc/hook.h"
#include "../inc/util.h"

#include <linux/module.h>

syscall_t *_getdents = NULL;
syscall_t *_getdents64 = NULL;

asmlinkage int64_t h_getdents(const struct pt_regs *r) {
  if (_getdents == NULL)
    _getdents = hooks_find_orig(__NR_getdents);

  return _getdents(r);
}

asmlinkage int64_t h_getdents64(const struct pt_regs *r) {
  if (_getdents64 == NULL)
    _getdents64 = hooks_find_orig(__NR_getdents64);

  return _getdents64(r);
}
