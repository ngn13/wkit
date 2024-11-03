#pragma once
#include "job.h"
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t cmd_t;

// helper functions (cmd.c)
bool __cmd_recv_all(job_t *job);
#define cmd_recv_all(j)                                                                                                \
  if (!__cmd_recv_all(j))                                                                                              \
  return NULL
bool cmd_handle(job_t *job);

// commands (cmds/)
char *cmd_list(job_t *job);
char *cmd_info(job_t *job);
char *cmd_shell(job_t *job);
char *cmd_chdir(job_t *job);
char *cmd_hide(job_t *job);
char *cmd_unhide(job_t *job);
char *cmd_delete(job_t *job);
char *cmd_run(job_t *job);
