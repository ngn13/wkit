#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef enum kernel_cmd {
  KERNEL_CMD_DESTRUCT = 'D',
  KERNEL_CMD_PROTECT  = 'P',
  KERNEL_CMD_UNHIDE   = 'U',
  KERNEL_CMD_CHECK    = 'C',
  KERNEL_CMD_HIDE     = 'H',
} kernel_cmd_t;

bool kernel_load();
bool kernel_unload();
bool kernel_send(kernel_cmd_t cmd, void *arg, uint64_t len);
