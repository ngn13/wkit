#pragma once
#include <linux/module.h>

typedef bool cmd_handler_t(char *arg, uint64_t len);

bool cmds_install(void);
void cmds_uninstall(void);

// command helpers (used by the syscalls)
bool is_process_protected(pid_t pid);
bool is_inode_protected(uint64_t inode);

// command handlers
cmd_handler_t cmd_protect;
