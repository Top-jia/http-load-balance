#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(void) {
	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
		exit(1);
	}
	else if (pid > 0)
	{
		printf("Parent\n");
		exit(0);
	}
	else{
		printf("Child\n");
	}
	return 0;
}
