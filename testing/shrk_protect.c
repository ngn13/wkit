#include "common.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if(argc != 2){
    puts("this tool protects a given PID");
    printf("usage: %s <pid>\n", argv[0]);
    return EXIT_FAILURE;
  }

  pid_t pid = atoi(argv[1]);
  char cmd[sizeof(pid)+1];

  cmd[0] = 'P';
  memcpy(cmd+1, &pid, sizeof(pid));

  if(!proc_write(cmd, sizeof(cmd))){
    perror("failed to write the command");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
