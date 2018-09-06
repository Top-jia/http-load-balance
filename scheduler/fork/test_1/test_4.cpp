#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define BUFF_SIZE 127
int main(){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	int pipefd[2];

	pipe(pipefd);
	pid_t pid = fork();
	if(pid == -1){
		printf("errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	else if(pid == 0){
		close(pipefd[0]);
		write(pipefd[1], "hello world", 11);
		close(pipefd[1]);
		sleep(1);
		exit(EXIT_SUCCESS);
	}
	else{
		close(pipefd[1]);
		read(pipefd[0], buffer, BUFF_SIZE-1);
		printf("%s\n", buffer);
		close(pipefd[0]);
		exit(EXIT_SUCCESS);
	}
}
