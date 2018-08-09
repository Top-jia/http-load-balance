#include"Json.hpp"

extern Logger log;
Sinfo::Sinfo(){

}

void Sinfo::SetIP(std::string ip){
	this->ip = ip;
}

void Sinfo::SetPort(int port){
	this->port = port;
}

void Sinfo::Show(){
	std::cout << "ip: " << ip << " port: " << port << std::endl;
}

Sinfo::Sinfo(const Sinfo &src){
	ip = src.ip;
	port = src.port;
}

void Sinfo::operator=(const Sinfo &src){
	
	if(&src != this){
		ip = src.ip;
		port = src.port;
	}
}

std::string Sinfo::GetIP(){
	return ip;
}

int Sinfo::GetPort(){
	return port;
}
/*****************************/
/*
 *	打开配置文件
 * */
Json::Json(){
	
	struct stat statbuff;
	memset(&statbuff, '\0', sizeof(statbuff));
	int stat_return = stat(etc_file, &statbuff);
	if(stat_return == -1){
		log.WriteFile(true, errno, "Json::Json stat() failed ");
	}

	int fd = open(etc_file, O_RDONLY);
	if(fd == -1){
		log.WriteFile(true, errno, "Json::Json open() failed ");
	}

	char buffer[statbuff.st_size+1];
	memset(buffer, '\0', statbuff.st_size+1);
	read(fd, buffer, statbuff.st_size);
	
	root = cJSON_Parse(buffer);
	close(fd);
}

/*
 *	解析Json配置文件
 * */
void Json::ParseConfigure(){
	if(root == NULL){
		log.WriteFile(true, 0, "Json::ParseConfigure() failed");
	}
	
	/* sche_info*/
	cJSON *sub = cJSON_GetObjectItem(root, "host_info");
	cJSON *sub_node = cJSON_GetObjectItem(sub, "ip");
	sche_info.SetIP(sub_node->valuestring);
	sub_node = cJSON_GetObjectItem(sub, "port");
	sche_info.SetPort(sub_node->valueint);

	/* ser_info*/
	cJSON *array = cJSON_GetObjectItem(root, "ser_info");
	int size = cJSON_GetArraySize(array);
	for(int i = 0; i < size; i++){
		sub = cJSON_GetArrayItem(array, i);
		sub_node = cJSON_GetObjectItem(sub, "ip");
		ser_info[i].SetIP(sub_node->valuestring);
		sub_node = cJSON_GetObjectItem(sub, "port");
		ser_info[i].SetPort(sub_node->valueint);
	}

	/* default_methon*/
	sub = cJSON_GetObjectItem(root, "default_methon");
	default_methon = cJSON_GetStringValue(sub);
}

/*
 *	显示json配置信息
 * */
void Json::ShowConfigure(){
	char *buffer = cJSON_Print(root);
	std::cout << "cJSON_Print() " << buffer << std::endl;
	free(buffer);
	
	std::cout << "show host_info " << std::endl;
	sche_info.Show();

	std::cout << "show ser_info  " << std::endl;
	for(int i = 0; i < 3; i++){
		ser_info[i].Show();
	}

	std::cout << "show default_methon " << default_methon <<std::endl;
}

/*
 *	销毁Json文件
 * */
Json::~Json(){
	cJSON_Delete(root);
}

Sinfo Json::GetSche_info(){
	return sche_info;
}

Sinfo Json::GetSer_info(int i){
	return ser_info[i];
}

std::string Json::GetMethon(){
	return default_methon;
}
