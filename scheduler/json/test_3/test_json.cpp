#include"cJSON.h"
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define location	"./my_json"


cJSON* my_parse(){
	struct stat statbuff;
	memset(&statbuff, '\0', sizeof(statbuff));	
	int stat_return = stat(location, &statbuff);
	if(stat_return == -1){
		std::cout << "my_parse()_stat failed errno = " << errno << " str_error = " << strerror(errno) << std::endl;
		exit(0);
	}

	int fd = open(location, O_RDWR | O_APPEND);
	if(fd == -1){
		std::cout << "my_parse()_open failed errno = " << errno << "str_error = " << strerror(errno) << std::endl;
		exit(0);
	}

	char buffer[statbuff.st_size + 1];
	memset(buffer, '\0', statbuff.st_size + 1);
	read(fd, buffer, statbuff.st_size);
	close(fd);

	return cJSON_Parse(buffer);
}

int main(){
	cJSON *root = my_parse();
	char *buffer = cJSON_Print(root);
	std::cout << buffer << std::endl;
	free(buffer);
	cJSON_Delete(root);
	return 0;
}
