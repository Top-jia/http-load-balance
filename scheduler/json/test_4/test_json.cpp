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

#define output	"self.json"

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

	char buffer[statbuff.st_size+1];
	memset(buffer, '\0', statbuff.st_size+1);
	read(fd, buffer, statbuff.st_size);
	close(fd);
	return cJSON_Parse(buffer);
}

/*
 *	{
 *		"name":		"王小二",
 *		"age":		25.2,
 *		"birthday":	"1990-01-01",
 *		"school":	"蓝翔",
 *		"major":	["理发", "挖掘机"],
 *		"has_grilfriend":	false,
 *		"car":			null,
 *		"house":		null,
 *		"comment":	"this is a comment",
 *		"employ":[
 *					{"firstname":	"Bill",	"lastname":	"Gates"},
 *					{"firstname":	"George",	"lastname":	"Bush"},
 *					{"firstname":	"Thomas",	"lastname":	"Carter"}
 *				]
 *	}
 * */
void self_parse(cJSON *root){
	assert(root);
	
	char *buffer = cJSON_Print(root);
	std::cout << "cJSON_Parse():: " << buffer << std::endl;
	free(buffer);

	cJSON *sub = cJSON_GetObjectItem(root, "name");
	std::cout << "name: " << sub->valuestring << std::endl;
	
	sub = cJSON_GetObjectItem(root, "age");
	std::cout << "age: " <<  sub->valuedouble << std::endl;

	sub = cJSON_GetObjectItem(root, "birthday");
	buffer = cJSON_GetStringValue(sub);
	std::cout << "birthday: " << buffer << std::endl;

	sub = cJSON_GetObjectItem(root, "school");
	std::cout << "school: " << sub->valuestring << std::endl;

	cJSON *array = cJSON_GetObjectItem(root, "major");
	int size = cJSON_GetArraySize(array);
	std::cout << "major length of array :" << size << std::endl;
	for(int i = 0; i < size; i++){
		sub = cJSON_GetArrayItem(array, i);
		std::cout << "index i = " << i << " elem = " << sub->valuestring << std::endl;
	}

	sub = cJSON_GetObjectItem(root, "has_grilfirend");
	cJSON_bool flag = cJSON_IsBool(sub);
	if(flag){
		flag = cJSON_IsFalse(sub);
		std::cout << "has_gitlfirend :" << flag << std::endl;
	}

	sub = cJSON_GetObjectItem(root, "car");
	flag = cJSON_IsNull(sub);
	if(flag){
		std::cout << "car : " << "null" << std::endl;
	}

	sub = cJSON_GetObjectItem(root, "house");
	flag = cJSON_IsNull(sub);
	if(flag){
		std::cout << "house : " << "null " << std::endl;
	}

	sub = cJSON_GetObjectItem(root, "comment");
	buffer = cJSON_GetStringValue(sub);
	std::cout << "comment: " << buffer << std::endl;

	sub = cJSON_GetObjectItem(root, "employ");
	size = cJSON_GetArraySize(sub);
	std::cout << "employ: length of array " << size << std::endl;
	for(int i = 0; i < size; i++){
		cJSON *sub_node = cJSON_GetArrayItem(sub, i);
		cJSON *sub_node_node = cJSON_GetObjectItem(sub_node, "firstname");
		std::cout << "firstname: " << sub_node_node->valuestring << std::endl;
		sub_node_node = cJSON_GetObjectItem(sub_node, "lastname");
		std::cout << "lastname : " << sub_node_node->valuestring << std::endl;
	}
}

int main(){
	cJSON *root = get_json_object();
	self_parse(root);
	cJSON_Delete(root);
	return 0;
}
