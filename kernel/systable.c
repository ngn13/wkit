#include "inc/systable.h"
#include "inc/hook.h"
#include "inc/util.h"

#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/syscalls.h>

uint64_t *systable = NULL; // stores the syscall table pointer

bool systable_hook(void) {
  uint64_t (*kln)(const char *name);
  struct kprobe kp = {.symbol_name = "kallsyms_lookup_name"};
  bool ret = false;

  // get the kallsysms_lookup_name
  register_kprobe(&kp);
  kln = (void *)kp.addr;
  unregister_kprobe(&kp);

  if (NULL == kln) {
    debg("failed to find kallsysms_lookup_name");
    return false;
  }

  // get the systable
  if ((systable = (void *)kln("sys_call_table")) == NULL) {
    debg("failed to find the syscall table");
    return false;
  }

  // hook da table
  clear_cr0_wp();
  ret = hooks_setup(systable);
  set_cr0_wp();

  return ret;
}

bool systable_unhook(void) {
  bool ret = false;

  if (NULL == systable)
    return false;

  // unhook da table
  clear_cr0_wp();
  ret = hooks_clean(systable);
  set_cr0_wp();

  return ret;
}
