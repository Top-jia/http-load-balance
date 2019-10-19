#ifndef JSON_HPP
#define JSON_HPP

#include"cJSON.h"
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include"../log/log.hpp"
#include<string>
#include<map>

#define etc_file "./etc/config.json" 

class Sinfo{
		std::string ip;
		int port;

	public:
		Sinfo();
		void SetIP(std::string ip);
		void SetPort(int port);
		void Show();

		Sinfo(const Sinfo &src);
		void operator=(const Sinfo &src);
		std::string GetIP();
		int GetPort();
};

class Json{
		Sinfo sche_info;
		Sinfo ser_info[3];
		cJSON *root;
		std::string default_methon;
	public:
		Json();
		void ParseConfigure();
		void ShowConfigure();
		Sinfo GetSche_info();
		Sinfo GetSer_info(int index);
		std::string GetMethon();

		std::map<std::string, std::string> GetSche();
		~Json();
};

#endif 
