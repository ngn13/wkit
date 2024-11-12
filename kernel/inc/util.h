#pragma once
#include <net/sock.h>

#if SHRK_DEBUG == 1
#define debg(msg)       print_debug(__func__, msg)
#define debgf(msg, ...) print_debug(__func__, msg, __VA_ARGS__)
#else
#define debg(msg)       asm("nop")
#define debgf(msg, ...) asm("nop")
#endif

void print_debug(const char *caller, char *msg, ...);
char *path_join(char *p1, char *p2);
char *path_from_fd(int32_t fd, char path[PATH_MAX+1]);
uint64_t inode_from_sock(struct sock *sk);
