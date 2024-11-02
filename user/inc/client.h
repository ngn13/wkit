#pragma once
#include <arpa/inet.h>
#include <stdbool.h>

typedef struct client {
  struct sockaddr server; // server address
  int             socket; // server socket
} client_t;

bool    client_setup(client_t *c, char *addr, uint16_t port);
int64_t client_send(client_t *c, char *data, uint64_t data_size);
int64_t client_recv(client_t *c, char *data, uint64_t data_size);
