#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include "util.h"

int ssend(int s, const char* msg){
  return send(s, msg, strlen(msg), 0);
}

int info(int s, const char* msg){
  char newmsg[strlen(msg)+30];
  sprintf(newmsg, BLUE BOLD"=> "RESET BOLD"%s\n"RESET, msg);
  send(s, newmsg, strlen(newmsg), 0);
}

int error(int s, const char* msg){
  char newmsg[strlen(msg)+30];
  sprintf(newmsg, RED BOLD"=> "RESET BOLD"%s\n"RESET, msg);
  send(s, newmsg, strlen(newmsg), 0);
}

int prompt(int s) {
  return ssend(s, BOLD RED"[wkit]# "RESET);
}

bool exists(char* path){ 
  return access(path, F_OK)==0;
}

char* recvline(int s){
  char* line = NULL;
  char cur[1] = "0";
  int size = 0;

  while(NULL==line || '\n'!=cur[0]){
    if(line != NULL)
      line[size-1] = cur[0];
    
    size++;
    if(line == NULL)
      line = malloc(size);
    else
      line = realloc(line, size);

    if(recv(s, cur, 1, 0)<=0)
      return NULL;
  }

  return line;
}
