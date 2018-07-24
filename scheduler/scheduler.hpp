#ifndef SCHEDULER
#define SCHEDULER

#include<iostream>
#include<string>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<errno.h>
#include<assert.h>
#include<stdbool.h>
#include<sys/epoll.h>
/*
 *	对于自己设置的头文件, 必须加载器头文件的位置
 * */
#include"./log/log.hpp"
#include"./bstage/bstage.hpp"
#include"./sepoll/sepoll.hpp"

#define MAX_EVENT_NUM	1024
#define BUFF_SIZE	127
#define _GUN_SOURCE
typedef  int FD;

class Scheduler{
		std::string ip;
		int port;
		Logger log;
		Bstage bstage;
		
		Sepoll sepoll;
		struct epoll_event sepoll_events[MAX_EVENT_NUM];
		FD  scheduler_fd;
	public:
		Scheduler();
		void CreateLink();
		void Run();
};

#endif
