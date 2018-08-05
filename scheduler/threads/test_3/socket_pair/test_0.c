#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<errno.h>
#include<error.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>


#define BUFF_SIZE	127

int main(){
	int sockpair[2];
	int w, r;
	char *string = "this is a test string";
	char *buffer = (char*)calloc(1, BUFF_SIZE);

	if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair) == -1){
		printf("socketpair create failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*
	 *	test in a single process
	 * */
	if(w = write(sockpair[0], string, strlen(string)) == -1){
		printf("write(s[0],, ,) failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*read*/
	if(r = read(sockpair[1], buffer, BUFF_SIZE) == -1){
		printf("read failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	printf("Read string in some process: %s\n", buffer);
	return 0;
}
