#include <sys/syscall.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "common.h"
  
// this tool destructs the kernel rookit

int main(int argc, char *argv[]) {
  char cmd[2];

  cmd[0] = 'D';
  cmd[1] = 1; // doesnt really matter

  if(!proc_write(cmd, sizeof(cmd))){
    perror("failed to write the command");
    return EXIT_FAILURE;
  }

  /*
 
   * rootkit is now visible in the module list
   * so we can just remove it

   * from delete_module manual notes:
   * > The  uninterruptible sleep that may occur if O_NONBLOCK is omitted from flags is considered undesirable,
   * > because the sleeping process is left in an unkillable state.  As at Linux 3.7, specifying O_NONBLOCK is optional, but
   * > in future kernels it is likely to become mandatory.

  */
  if(syscall(SYS_delete_module, "shrk", O_NONBLOCK) != 0){
    perror("failed to remove the module");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
