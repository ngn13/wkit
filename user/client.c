#include "inc/client.h"
#include "inc/util.h"

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <netdb.h>
#include <stdio.h>

// timeout option for the socket (10 secs)
const struct timeval timeout = {
    .tv_sec  = 10,
    .tv_usec = 0,
};

bool client_setup(client_t *c, char *addr, uint16_t port) {
  if (NULL == c || NULL == addr) {
    errno = EINVAL;
    return false;
  }

  struct addrinfo info;
  bool            ret = false;

  bzero(&info, sizeof(info));
  resolve(&info, &c->server, addr, port);

  // create the socket
  if ((c->socket = socket(info.ai_family, SOCK_DGRAM, 0)) < 0)
    goto end;

  // set socket timeout
  if (setsockopt(c->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    debug("setsockopt failed for SO_RCVTIMEO: %s", strerror(errno));
    return false;
  }

  if (setsockopt(c->socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
    debug("setsockopt failed for SO_SNDTIMEO: %s", strerror(errno));
    return false;
  }

  // print debug shit if needed
  if (SHRK_DEBUG) {
    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(info.ai_family,
        info.ai_family == AF_INET ? (void *)&((struct sockaddr_in *)info.ai_addr)->sin_addr
                                  : (void *)&((struct sockaddr_in6 *)info.ai_addr)->sin6_addr,
        ipstr,
        INET6_ADDRSTRLEN);
    debug("created client for %s:%d (family %d)",
        ipstr,
        info.ai_family == AF_INET ? ntohs(((struct sockaddr_in *)&c->server)->sin_port)
                                  : ntohs(((struct sockaddr_in6 *)&c->server)->sin6_port),
        info.ai_family);
  }

  ret = true;
end:
  if (!ret && c->socket != -1)
    close(c->socket);
  return ret;
}

int64_t client_send(client_t *c, char *data, uint64_t data_size) {
  if (NULL == data || NULL == c) {
    errno = EINVAL;
    return -1;
  }

  return sendto(c->socket, data, data_size, 0, &c->server, sizeof(struct sockaddr));
}

int64_t client_recv(client_t *c, char *data, uint64_t data_size) {
  if (NULL == data || NULL == c) {
    errno = EINVAL;
    return -1;
  }

  uint32_t addrlen = sizeof(struct sockaddr);
  return recvfrom(c->socket, data, data_size, 0, &c->server, &addrlen);
}
