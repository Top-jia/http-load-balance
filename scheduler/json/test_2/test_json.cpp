#include"cJSON.h"
#include<unistd.h>
#include<stdlib.h>
#include<iostream>
#include<string.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(){
	
	const char *str = "{\"habit\": [\"LOL\", \"Go shopping\", \"Sport\"], \"subject\": \"Learn\"}";
	cJSON *root = cJSON_Parse(str);

	char *buffer = cJSON_Print(root);
	std::cout << buffer << std::endl;

	int fd = open("output_json.txt", O_CREAT | O_RDWR | O_APPEND, 0666);
	assert(fd != -1);
	write(fd, buffer, strlen(buffer));
	close(fd);
	
	free(buffer);
	cJSON_Delete(root);
	return 0;
}
