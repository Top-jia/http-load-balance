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
	public:
		Sepoll();
		static void setnonblocking(FD fd);
		
		void addfd(FD fd, struct epoll_event *event = NULL);
		void delfd(FD fd);
		void modfd(FD fd, struct epoll_event *event);
		FD getFD();
};
#endif
