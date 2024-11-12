#include "common.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if(argc != 2){
    puts("this tool hides a given file/dir");
    printf("usage: %s <path>\n", argv[0]);
    return EXIT_FAILURE;
  }

  size_t len = strlen(argv[1]);
  char cmd[len+1];

  cmd[0] = 'H';
  memcpy(cmd+1, argv[1], len);

  if(!proc_write(cmd, sizeof(cmd))){
    perror("failed to write the command");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
