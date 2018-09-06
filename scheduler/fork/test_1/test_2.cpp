#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

#define BUFF_SIZE 125
int main(){

	FILE *fptr = NULL;
	char buffer[BUFF_SIZE];
	fptr = popen("ps -ax", "r");
	sprintf(buffer, "%s", "hello world!");
	if(fptr){
		while(true){
		int n = fread(buffer, sizeof(char), BUFF_SIZE - 1, fptr);
		if(n <= 0){
				break;
			}
			printf("%s", buffer);
			memset(buffer, '\0', BUFF_SIZE);
		}
	}
	exit(EXIT_SUCCESS);
}
