#include "inc/util.h"

#include <linux/module.h>
#include <linux/slab.h>

void print_debug(const char *caller, char *msg, ...) {
  char *fmt = NULL;
  va_list args;

  if (!SHRK_DEBUG)
    return;

  fmt = kmalloc(strlen(msg) + strlen(caller) + 12, GFP_KERNEL);
  sprintf(fmt, "[shrk] %s: %s\n", caller, msg);

  va_start(args, msg);

  vprintk(fmt, args);

  va_end(args);
  kfree(fmt);
}
