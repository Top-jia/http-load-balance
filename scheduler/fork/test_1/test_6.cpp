#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define BUFF_SIZE 127
int main(int argc, char *argv[]){

	int fd = -1;
	sscanf(argv[1], "%d", &fd);
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	read(fd, buffer, BUFF_SIZE-1);
	printf("buffer = %s\n", buffer);
	exit(23);
}
