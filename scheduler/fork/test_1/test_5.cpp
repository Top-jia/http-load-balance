#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>
#define BUFF_SIZE	127

int main(int argc, char *argv[]){
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
		sprintf(buffer, "%d", pipefd[0]);
		close(pipefd[1]);
		execl(argv[1], argv[1], buffer, (char*)0);
		printf("execl failure errno = %d str_error(errno) = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	else{
		close(pipefd[0]);
		write(pipefd[1], "hello world", 11);
		close(pipefd[1]);
		int stat_val;
		pid_t child_pid;
		if((child_pid = wait(&stat_val)) == -1){
			printf("errno = %d str_err = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if(WIFEXITED(stat_val)){
			printf("child exit_code = %d, pid = %d\n", WEXITSTATUS(stat_val), child_pid);
		}
	}
	exit(EXIT_SUCCESS);
}
