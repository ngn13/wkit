#pragma once
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#define RED     "\x1b[31m"
#define BOLD    "\x1b[1m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"

int error(int, const char*);
int ssend(int, const char*);
int info(int, const char*);
char* recvline(int);
bool exists(char*); 
int prompt(int);
