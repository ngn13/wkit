#include "../inc/hook.h"
#include <linux/module.h>

asmlinkage int64_t h_getdents(const struct pt_regs *) { return 0; }

asmlinkage int64_t h_getdents64(const struct pt_regs *) { return 0; }
