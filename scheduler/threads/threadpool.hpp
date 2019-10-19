#include<iostream>
#include<stdio.h>
#include<pthread.h>
#include<assert.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<vector>

#include <unistd.h>

#include"../json/Json.hpp"
#include"../locker/locker.hpp"
#include"../log/log.hpp"
#include"../sepoll/sepoll.hpp"



#define BUFF_SIZE 127
#define MAX_THREAD_NUM	10
#define EVENT_NUM	1024
#define FILE_PATH	"./web/index.html"

class Threadpool{
	public:
		Threadpool(int thread_num, int pipefd);
		~Threadpool();
		void* thread_funtion(void *arg);
		/*获取json中配置网络链接的ip和端口信息*/
		std::vector<Sinfo> ser_info;
		/*
		 *	封装线程处理函数
		 * */
		static void* run(void *arg);
	private:
		pthread_t *pthread_array;
		int pthread_num;
		/*
		 *	这个管道是主线程和子线程用来通信的fd,设置非阻塞模式,
		 * */
		int pipe_read;
		bool sum_flags;
		/*
		 *	线程内部处理的函数->可以封装成一个类
		 *是对其中的epool数据进行分装, 数据进行处
		 *理的一个内部处理类
		 * */
			typedef 
			class Epolldata{
				public:	
					Sepoll sepoll;
					struct epoll_event events[EVENT_NUM];
					//一种状态的, 来标识线程的执行
					bool sub_flags;
					 Epolldata();
					~Epolldata();
					void process_pipe_data(int fd);
					void close_fd(int fd);
					void process_data(int fd, std::vector<Sinfo> &);

					/*处理其他信息*/
					void reply_http_info(int fd, std::vector<Sinfo > &);
			}Epdata;
};

void reply_http(int fd);
