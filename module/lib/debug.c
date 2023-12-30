#include <linux/module.h>
#include <linux/slab.h>
#include "config.h"

void print(char* msg, ...){
	va_list fmt;
	char* edited;

	if(!in_debug) return;

	edited = kmalloc(strlen(msg)+10, GFP_KERNEL);
	sprintf(edited, "[wkit] %s\n", msg);

	va_start(fmt, msg);
	vprintk(edited, fmt);
	va_end(fmt);
	
	kfree(edited);
}
