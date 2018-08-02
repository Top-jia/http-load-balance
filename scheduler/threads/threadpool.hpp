#include<iostream>
#include<stdio.h>
#include<pthread.h>
#include<assert.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include"locker.hpp"
#include"../log/log.hpp"
#include"../sepoll/sepoll.hpp"

#define BUFF_SIZE 127
#define MAX_THREAD_NUM	10
#define EVENT_NUM	1024

class Threadpool{
	public:
		Threadpool(int thread_num, int pipefd);
		~Threadpool();
		void* thread_funtion(void *arg);
		
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
		int pipe_fd;
		Logger log;
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
				/*
				 *	一种状态的
				 * */
				bool sub_flags;
				Sem sem;
				Logger log;

				Epolldata(Logger &tmp);
				~Epolldata();
				void process_pipe_data(int fd);
				void close_fd(int fd);
				void process_data(int fd);
		}Epdata;
};
