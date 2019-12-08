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
#include<unistd.h>
/*
 *	对于自己设置的头文件, 必须加载器头文件的位置
 * */
#include"./log/log.hpp"
#include"./bstage/bstage.hpp"
#include"./sepoll/sepoll.hpp"
#include"./json/Json.hpp"
#include"./threads/threadpool.hpp"
#include"./locker/locker.hpp"

#define MAX_EVENT_NUM	1024
#define BUFF_SIZE	127
#define _GUN_SOURCE

#define SER_NUM		3

extern Logger log;
typedef  int FD;
class Scheduler{
		Bstage bstage;
		Sinfo sche_info;
		/*将网络配置文件放在线程处理函数中*/
		//Sinfo ser_info[SER_NUM];
		std::string methon;

		Sepoll sepoll;
		struct epoll_event sepoll_events[MAX_EVENT_NUM];
		FD  scheduler_fd;
		/*
		 *	子线程和主线程通信的工具
		 * */
		FD sockpair[2];
		/*一些处理数据的函数*/
		void accept_link(int fd);
		/*增加写入文件描述符的长度*/
		void addBufferLen(char *buffer);
	public:
		Scheduler();
		void CreateLink();
		void Run();
};
#endif
