#include"cJSON.h"
#include<string.h>
#include<iostream>
#include<assert.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>

#define output	"./output.json"
#define input	"./input.json"

cJSON *get_json_object(){
	struct stat statbuff;
	memset(&statbuff, '\0', sizeof(statbuff));
	int stat_return = stat(output, &statbuff);
	if(-1 == stat_return){
		std::cout << "get_json_object:: stat failed errno = " << errno << \
			" strerror = " << strerror(errno) << std::endl;
		exit(0);
	}

	int fd = open(output, O_RDWR);
	assert(fd != -1);

	char buffer[statbuff.sz_size+1];
	memset(buffer, '\0', statbuff.sz_size+1);
	read(fd, buffer, statbuff.sz_buff);
	close(fd);
	return cJSON_Parse(buffer);
}

void self_parse(cJSON *root){
	assert(root);
	

}

int main(){
	cJSON *root = get_json_object();
	self_parse(root);
	return 0;
}
