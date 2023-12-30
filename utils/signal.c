#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv){
	if(argc != 3){
		printf("usage: %s <pid> <signal>\n", argv[0]);
		return EXIT_SUCCESS;
	}

	int pid = atoi(argv[1]);
	int sig = atoi(argv[2]);

	if(pid <= 0){
		printf("bad pid\n");
		return EXIT_FAILURE;
	}

	kill(pid, sig);
	return EXIT_SUCCESS;
}
