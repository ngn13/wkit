#pragma once
#include "../inc/config.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "util.h"

int cmd_shell(int); 
int cmd_hide(int);
int cmd_pid(int);
