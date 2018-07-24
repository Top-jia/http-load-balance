#ifndef LOGGER
#define LOGGER
#include<iostream>
#include<assert.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<fcntl.h>
#include<stdbool.h>

/*
 *	日志文件的存放位置
 * */
#define LOG_FILE	"./log_file.txt"

#define BUFF_SIZE	127

class Logger{
	std::string log_place;
	public:
		Logger();
		std::string GetCurTime();
		/*
		 *	errno_level:
		 *		false	为不影响程序运行的警告错误
		 *		true	直接将程序终止, 如系统调用的错误.
		 *	location:
		 *		出错的函数位置信息
		 *	errno_num:
		 *		出错的号码, 和出错的信息.
		 * */
		void WriteFile(bool errno_level, int errno_num, std::string location);
};

#endif
