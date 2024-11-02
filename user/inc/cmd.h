#pragma once
#include "job.h"
#include <stdbool.h>
#include <stdint.h>

enum cmd {
  CMD_INFO  = 'I',
  CMD_SHELL = 'S',
  CMD_CHDIR = 'C',
  CMD_LIST  = 'L',
};

typedef uint8_t cmd_t;

bool cmd_handle(job_t *job);
