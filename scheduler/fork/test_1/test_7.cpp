#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>

#define BUFF_SIZE	127
int main(){
	int pipefd[2];
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);

	int err_info;
	if((err_info = pipe(pipefd)) != 0){
		printf("pipe failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if(pid == -1){
		printf("fork failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	else if(pid == 0){
		close(pipefd[0]);
		int write_fd = dup(pipefd[1]);
		close(pipefd[1]);
		write(write_fd, "hello world", 11);
		close(write_fd);
		exit(EXIT_SUCCESS);
	}
	else{
		close(pipefd[1]);
		int fd = dup2(pipefd[0], 10);
		close(pipefd[0]);
		read(fd, buffer, BUFF_SIZE-1);
		printf("buffer = %s, fd = %d\n", buffer, fd);
		close(fd);
	}
	exit(EXIT_SUCCESS);
}
