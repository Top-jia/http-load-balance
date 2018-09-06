#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>

#define BUFF_SIZE 127
int main(){
	int pipefd[2];
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	int err_info = pipe(pipefd);
	if(err_info != 0){
		printf("errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	write(pipefd[1], "hello world", 11);
	read(pipefd[0], buffer, BUFF_SIZE);
	close(pipefd[0]);
	close(pipefd[1]);
	printf("buffer = %s\n", buffer);
	exit(EXIT_SUCCESS);
}
