#include"cJSON.h"
#include<unistd.h>
#include<stdlib.h>
#include<iostream>
#include<string>

int main()
{
	const char *data = "{\"love\":[\"LOL\",\"Go shopping\"]}";
	cJSON *json = cJSON_Parse(data);
	
	char *json_data = NULL;
	json_data = cJSON_Print(json);
	
	std::cout << json_data << std::endl;
	free(json_data);
	cJSON_Delete(json);
	return 0;
}
