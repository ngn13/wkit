#include "../inc/cmds.h"
#include "../inc/util.h"

#include <linux/module.h>

bool cmd_destruct(char *arg, uint64_t len) {
  showself();
  return true;
}
