#pragma once

#define debg(msg) print_debug(__func__, msg)
#define debgf(msg, ...) print_debug(__func__, msg, __VA_ARGS__)

void print_debug(const char *caller, char *msg, ...);
void set_cr0_wp(void);
void clear_cr0_wp(void);
