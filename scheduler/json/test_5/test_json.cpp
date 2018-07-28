#include"cJSON.h"
#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<string.h>

#define  location "self.json"
/*
 *{
 	"name":		"王小二",
	"age":		25.2,
	"birthday":	"1990-01-01",
	"school":		"蓝翔",
	"major":		["理发", "挖掘机"],
	"has_grilfriend":	false,
	"grade":		[78.5, 23.5, 89.1],
	"car":			null,
	"house":		null,
	"pass_grade":	["math": true, "PE": false, "English": true],
	"comment":	"this is a comment",
	"employ":[
				{"firstname":	"Bill",	"lastname":	"Gates"},
				{"firstname":	"George",	"lastname":	"Bush"},
				{"firstname":	"Thomas",	"lastname":	"Carter"}
			]
	}
 * */

cJSON* create_json(){
	cJSON *root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("王小二"));
	cJSON_AddNumberToObject(root, "age", 25.2);
	cJSON_AddStringToObject(root, "birthday", "1990-01-01");
	cJSON *sub  = NULL;
	cJSON_AddItemToObject(root, "school", sub = cJSON_CreateString("蓝翔"));
	
	const char *chars[2] = {"理发", "挖掘机"};
	cJSON *array = cJSON_CreateStringArray(chars, sizeof(chars)/sizeof(chars[0]));
	cJSON_AddItemToObject(root, "major", array);
	
	cJSON_AddFalseToObject(root, "has_girlfriend");
	
	double double_array[3] = {73.8, 23.5, 89.1};
	cJSON_AddItemToObject(root, "grade", array = cJSON_CreateDoubleArray(double_array, sizeof(double_array)/sizeof(double_array[0])));

	cJSON_AddBoolToObject(root, "car", false);
	cJSON_AddBoolToObject(root, "house", false);
	
	array = cJSON_CreateArray();
	sub = cJSON_CreateObject();
	cJSON_AddTrueToObject(sub, "math");
	cJSON_AddItemToArray(array, sub);

	sub = cJSON_CreateObject();
	cJSON_AddFalseToObject(sub, "PE");
	cJSON_AddItemToArray(array, sub);

	sub = cJSON_CreateObject();
	cJSON_AddTrueToObject(sub, "English");
	cJSON_AddItemToArray(array, sub);
	cJSON_AddItemToObject(root, "grade_pass", array);

	cJSON_AddStringToObject(root, "comment", "this is a comment");

	array = cJSON_CreateArray();
	sub = cJSON_CreateObject();
	cJSON_AddItemToObject(sub, "firstname", cJSON_CreateString("Bill"));
	cJSON_AddStringToObject(sub, "lastname", "Gates");
	cJSON_AddItemToArray(array, sub);

	sub = cJSON_CreateObject();
	cJSON_AddItemToObject(sub, "firstname", cJSON_CreateString("Georeg"));
	cJSON_AddStringToObject(sub, "lastname", "Bush");
	cJSON_AddItemToArray(array, sub);

	sub = cJSON_CreateObject();
	cJSON_AddItemToObject(sub, "firstname", cJSON_CreateString("Thomas"));
	cJSON_AddStringToObject(sub, "lastname", "carter");
	cJSON_AddItemToArray(array, sub);

	cJSON_AddItemToObject(root, "employ", array);
	return root;
}


void write_file(cJSON *root){
	if(root == NULL){
		return ;
	}
	char *buffer = cJSON_Print(root);
	std::cout << "my.json " << buffer << std::endl;

	int fd = open(location, O_CREAT | O_RDWR, 0644);
	assert(fd != -1);
	write(fd, buffer, strlen(buffer));
	close(fd);
	free(buffer);
	cJSON_Delete(root);
}

int main(){
	cJSON *root = create_json();
	write_file(root);
	return 0;
}
