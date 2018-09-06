#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

#define BUFF_SIZE 125
int main(){
	FILE *fptr = NULL;
	char buffer[BUFF_SIZE];
	fptr = popen("uname -a", "r");
	if(fptr){
		int n = fread(buffer, sizeof(char), BUFF_SIZE - 1, fptr);
		if( n > 0){
			printf("Output: %s", buffer);
			fflush(stdin);
			fflush(stdout);
		}
		fclose(fptr);
	}
	exit(EXIT_SUCCESS);
}
