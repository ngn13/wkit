#include "inc/util.h"

#include <linux/module.h>

void print_debug(const char *caller, char *msg, ...) {
  char *fmt = NULL;

  if (!SHRK_DEBUG)
    return;

  fmt = kmalloc(strlen(msg) + strlen(caller) + 12, GFP_KERNEL);
  sprintf(fmt, "[shrk] %s: %s\n", caller, msg);

  va_list args;
  va_start(args, msg);

  vprintk(fmt, args);

  va_end(args);
  kfree(fmt);
}

/*

  * https://en.wikipedia.org/wiki/Control_register#Control_registers_in_Intel_x86_series
  * functions to disbale/enable write protection in priv level 0 (ring 0) needed
  bc the memory page containing
  * the syscall table is set as read-only, which is done by clearing R/W to in
  the page table entry

  * https://wiki.osdev.org/Paging:
  *   R/W, the 'Read/Write' permissions flag. If the bit is set, the page is
  read/write.
  *   Otherwise when it is not set, the page is read-only. The WP bit in CR0
  determines if this is only applied to userland,
  *   always giving the kernel write access (the default) or both userland and
  the kernel (see Intel Manuals 3A 2-20).

*/
void set_cr0_wp(void) {
  uint64_t cr0 = read_cr0();
  cr0 |= 1 << 16;
  asm volatile("mov %0,%%cr0" : "+r"(cr0) : __FORCE_ORDER);
}

void clear_cr0_wp(void) {
  uint64_t cr0 = read_cr0();
  cr0 &= ~(1 << 16);
  asm volatile("mov %0,%%cr0" : "+r"(cr0) : __FORCE_ORDER);
}
