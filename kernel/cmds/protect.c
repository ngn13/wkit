#include "../inc/cmds.h"
#include "../inc/util.h"

#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>

struct protect_status {
  pid_t   *list;
  uint64_t count;
} pst = {
    .list  = NULL,
    .count = 0,
};

bool cmd_protect(char *arg, uint64_t len) {
  if (len != sizeof(pid_t))
    return false;

  if (NULL == pst.list)
    pst.list = (void *)kmalloc(++pst.count * sizeof(pid_t), GFP_KERNEL);
  else
    pst.list = (void *)krealloc(pst.list, ++pst.count * sizeof(pid_t), GFP_KERNEL);

  memcpy(&pst.list[pst.count - 1], arg, len);
  return true;
}

struct task_struct *__cmd_protect_find(pid_t p) {
  struct task_struct *cur = NULL;

  for_each_process(cur) {
    if (cur->pid == p)
      return cur;
  }

  return NULL;
}

bool __cmd_protect_is_parent_protected(pid_t pid) {
  struct task_struct *tp = NULL;

  if ((tp = __cmd_protect_find(pid)) == NULL)
    return false;

  if (tp->real_parent->pid == 0)
    return false;

  return is_process_protected(tp->real_parent->pid);
}

bool is_process_protected(pid_t pid) {
  uint64_t i = 0;

  for (; i < pst.count; i++) {
    if (pst.list[i] == pid)
      return true;
  }

  if (__cmd_protect_is_parent_protected(pid))
    return true;

  return false;
}

bool __cmd_protect_is_inode_protected(struct task_struct *tp, uint64_t inode) {
  struct task_struct *child = NULL;
  struct file        *fp    = NULL;
  uint32_t            i     = 0;

  if (NULL == tp->files || NULL == tp->files->fdt)
    goto check_children;

  /*

   * heres the structures we go through to access to the file
   * descriptors (which obv includes inodes)

   * struct task_struct (linux/sched.h)
   * struct files_struct (linux/fdtable.h)
   * struct fdtable (linux/fdtable.h)
   * struct file (linux/fs.h)

  */
  for (; i < tp->files->fdt->max_fds; i++) {
    if (NULL == (fp = tp->files->fdt->fd[i]) || NULL == fp->f_inode)
      continue;

    if (fp->f_inode->i_ino == inode)
      return true;
  }

check_children:
  // also check the child procceses
  list_for_each_entry(child, &tp->children, sibling) {
    if (__cmd_protect_is_inode_protected(child, inode))
      return true;
  }

  return false;
}

bool is_inode_protected(uint64_t inode) {
  struct task_struct *tp = NULL;
  uint64_t            i  = 0;

  for (; i < pst.count; i++) {
    if ((tp = __cmd_protect_find(pst.list[i])) == NULL) {
      debgf("task struct not found for pid: %d", pst.list[i]);
      continue;
    }

    if (__cmd_protect_is_inode_protected(tp, inode))
      return true;
  }

  return false;
}
