#include "../inc/cmds.h"
#include "../inc/util.h"

#include <linux/sched/signal.h>
#include <linux/slab.h>

struct protect_status {
  pid_t *list;
  uint64_t count;
} pst = {
  .list = NULL,
  .count = 0,
};

bool cmd_protect(char *arg, uint64_t len){
  if(len != sizeof(pid_t))
    return false;

  if(NULL == pst.list)
    pst.list = (void*)kmalloc(++pst.count*sizeof(pid_t), GFP_KERNEL);
  else
    pst.list = (void*)krealloc(pst.list, ++pst.count*sizeof(pid_t), GFP_KERNEL);

  memcpy(&pst.list[pst.count-1], arg, len);
  return true;
}

struct task_struct *__cmd_protect_find(pid_t p){
  struct task_struct *cur = NULL;

  for_each_process(cur) {
    if (cur->pid == p)
      break;
  }

  return cur;
}

bool __cmd_protect_is_parent_protected(pid_t p){
  struct task_struct *proc = NULL;

  if((proc = __cmd_protect_find(p)) == NULL)
    return false;

  if(proc->real_parent->pid == 0)
    return false;

  return cmd_protect_is_protected(proc->real_parent->pid);
}

bool cmd_protect_is_protected(pid_t p){
  uint64_t i = 0;

  for(;i < pst.count; i++){
    if(pst.list[i] == p)
      return true;
  }

  if(__cmd_protect_is_parent_protected(p))
    return true;

  return false;
}
