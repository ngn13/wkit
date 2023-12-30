#pragma once
#include <linux/init.h>
#include <linux/module.h>
#include "util.h"

typedef asmlinkage long (*syscall)(const struct pt_regs *);

struct Call {
	int code;
	char* name;
	syscall org;
	syscall hook;
};

extern struct Call calls[];
bool clean_calls(unsigned long*);
bool setup_calls(unsigned long*);
syscall find_org(char*);
