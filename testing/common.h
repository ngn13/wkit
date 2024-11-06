#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SHRK_CLIENT_ID "DEBUGID0"
#define SHRK_PROC_FILE "/proc/shrk_" SHRK_CLIENT_ID

bool proc_write(char *cmd, uint64_t len) {
  FILE *pf = NULL;
  bool ret = false;

  if((pf = fopen(SHRK_PROC_FILE, "w")) == NULL)
    return ret;

  ret = fwrite(cmd, sizeof(char), len == 0 ? strlen(cmd) : len, pf) == len;
  fclose(pf);

  return ret;
}
