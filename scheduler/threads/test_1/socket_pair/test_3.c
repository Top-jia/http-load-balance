#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<error.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<errno.h>
#include<stdbool.h>
#include<fcntl.h>
#define BUFF_SIZE	127

void setnonblocking(int fd){
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
}

int main(){
	int sockpair[2];
	int w, r;
	char *string = "this is a test string";
	char *buffer = (char*)calloc(1, BUFF_SIZE);

	if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair) == -1){
		printf("socketpair create failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	pid_t pid = fork();
	if(pid == -1){
		printf("fork() create failure errno = %d strerror = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	else if(pid == 0){
		close(sockpair[0]);
		char buffer[BUFF_SIZE];
		memset(buffer, '\0', BUFF_SIZE);
		while(true){
			printf("Input data:\n");
			fgets(buffer, BUFF_SIZE-1, stdin);
			write(sockpair[1], buffer, strlen(buffer));
			memset(buffer, '\0', BUFF_SIZE);
		}
	}
	else{
		close(sockpair[1]);
		char buffer[BUFF_SIZE];
		setnonblocking(sockpair[0]);
		memset(buffer, '\0', BUFF_SIZE);
		while(true){
			read(sockpair[0], buffer, BUFF_SIZE-1);
			printf("buffer = %s", buffer);
			fflush(stdout);
			memset(buffer, '\0', BUFF_SIZE);
		}
	}

	return 0;
}
