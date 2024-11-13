#include <sys/syscall.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"

int main(int argc, char *argv[]) {
  if(argc != 2){
    puts("this tool loads the provided rookit module and drops you to a (protected) shell");
    puts("this way you can easily interact with the rookit without the debug mode");
    printf("usage: %s <path>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *args[] = {"/bin/bash", NULL};
  int module_fd = 0, err = 0;

  if((module_fd = open(argv[1], O_RDONLY)) < 0){
    perror("failed to open the provided module");
    return EXIT_FAILURE;
  } 

  err = syscall(SYS_finit_module, module_fd, "", 0);
  close(module_fd);

  if(err < 0){
    perror("failed to load the provided module");
    return EXIT_FAILURE;
  }
  
  execv(args[0], args);

  perror("failed to execute bash");
  return EXIT_FAILURE;
}
