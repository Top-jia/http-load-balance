#ifndef SEPOLL
#define SEPOLL

#include"../log/log.hpp"
#include<sys/epoll.h>
#include<string>
#include<fcntl.h>
#include<sys/types.h>
#include<unistd.h>

typedef int FD;
class Sepoll{
		FD epoll_fd;
		Logger &log;
	public:
		Sepoll(Logger &_log);
		
		void addfd(FD fd);
		void delfd(FD fd);
		void modfd(FD fd, struct epoll_event *event);
		void setnonblocking(FD fd);
		FD getFD();
};

#endif
