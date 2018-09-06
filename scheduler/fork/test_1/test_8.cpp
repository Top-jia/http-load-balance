#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/stat.h>
#include<stdbool.h>
#include<assert.h>
#include<fcntl.h>
#define BUFF_SIZE	127

#define PIPE_NAME	"mypipe"
int main(int argc, char *argv[]){
	if(argc <= 1){
		int pipe_ret = mkfifo(PIPE_NAME, 777);
		assert(pipe_ret != 0);
	}
	int fd = open(PIPE_NAME, O_RDONLY);
	if(fd == -1){
		printf("open() failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	while(true){
		int ret = read(fd, buffer, BUFF_SIZE-1);
		if(ret < 0){
			printf("read() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		printf("read_data = %s", buffer);
		fflush(stdout);
		memset(buffer, '\0', strlen(buffer));
	}
	return 0;
}
