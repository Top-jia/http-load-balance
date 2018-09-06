#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

#define BUFF_SIZE 125
int main(){
	FILE *fptr = NULL;
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	fptr = popen("od -c", "w");
	sprintf(buffer, "%s", "hello world!");
	if(fptr){
		int n = fwrite(buffer, sizeof(char), BUFF_SIZE - 1, fptr);
		fclose(fptr);
	}
	exit(EXIT_SUCCESS);
}
