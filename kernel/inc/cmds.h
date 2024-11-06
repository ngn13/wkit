#pragma once
#include <linux/module.h>

typedef bool cmd_handler_t(char *arg, uint64_t len);

bool cmds_install(void);
void cmds_uninstall(void);

// command helpers (used by the syscalls)
bool cmd_protect_is_protected(pid_t p);

// command handlers
cmd_handler_t cmd_protect;
