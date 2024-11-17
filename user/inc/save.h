#pragma once
#include <stdbool.h>

bool  save_creat();
bool  save_open();
char *save_get(char *name);
bool  save_add(char *name, char *value);
bool  save_del(char *name, char *value);
void  save_close();
void  save_remove();
