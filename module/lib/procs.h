#pragma once
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include "../../inc/config.h"
#include "util.h"


extern int* pids;
extern int pid_size;

void setuserpid(int);
void add_pid(int);
bool check_file(char*);
bool check_pid(int);
bool check_process(void);
void clean_pids(void);
