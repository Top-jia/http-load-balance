#include"threadpool.hpp"

extern Logger log;
Sem sem;
/*
 *	线程池的初始化
 * */
Threadpool::Threadpool(int thread_num, int pipefd):pthread_num(thread_num), pipe_read(pipefd), sum_flags(true){
	if(thread_num <= 0 || thread_num > MAX_THREAD_NUM){
		log.WriteFile(true, 0, "Threadpool::Threadpool failed in args");
	}
	/*
	 *	运行服务器并连接----
	 * */
	pthread_array = new pthread_t[thread_num];
	if(pthread_array == NULL){
		log.WriteFile(true, 0, "Threadpool::Threadpool failed in pthread_array ");
	}
	/*
	 *	创建多个线程
	 * */
	for(int i = 0; i < pthread_num; i++){
		if(pthread_create(pthread_array+i, NULL, run, (void*)this) !=  0){
			log.WriteFile(true, i, "Threadpool::Threadpool failed in pthread_create: ");
		}	
		/*
		 *	将其进行分开
		 */ 
		if(pthread_detach(pthread_array[i])){
			log.WriteFile(true, i, "Threadpool::Threadpool failed in pthread_detach: ");
		}
	}
}

/*
 *	封装线程处理函数
 * */
void* Threadpool::run(void *arg){
	Threadpool *pool = (Threadpool*)arg;
	pool->thread_funtion(arg);
}

/*
 *	释放整个线程池
 * */
Threadpool::~Threadpool(){

	delete [] pthread_array;
	sum_flags = false;
}

/*
 *  线程处理函数
 * */
void* Threadpool::thread_funtion(void *arg){
	Threadpool *mythis = (Threadpool*)arg;
	Epdata epdata;
	epdata.sepoll.setnonblocking(mythis->pipe_read);
	epdata.sepoll.addfd(mythis->pipe_read);

	while(epdata.sub_flags || mythis->sum_flags){
		
		int epoll_num = epoll_wait(epdata.sepoll.getFD(), epdata.events, EVENT_NUM, -1);
		if(epoll_num == -1){
			log.WriteFile(true, errno, "Threadpool::thread_funtion_epoll_wait failed ");
		}
		else if(epoll_num == 0){
			/*
			 *	这种情况是错误的, 不会出现的
			 * */
			continue;
		}
		else
		{
			for(int i = 0; i < epoll_num; i++){
				int fd = epdata.events[i].data.fd;
				/*
				 *	对管道数据进行处理
				 *先不约定对数据规则的定义,
				 * */
				if(fd == mythis->pipe_read){
					epdata.process_pipe_data(fd);
				}
				/*
				 *	对客户端关闭事件, 进行处理
				 * */
				else if(epdata.events[i].events & EPOLLRDHUP){
					epdata.close_fd(fd);
				}
				/*
				 *	对客户端的数据进行处理(非阻塞模式下)
				 * */
				else if(epdata.events[i].events & EPOLLIN){
					epdata.process_data(fd);
				}
				/*
				 *	其他事件的处理
				 * */
				else{
					std::cout << "otherthing will happened " << std::endl;
				}	
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 *	对内部类进行初始化
 * */
Threadpool::Epolldata::Epolldata():sub_flags(false){
	memset(events, '\0', sizeof(struct epoll_event) * EVENT_NUM);
}

/*
 *	对内部类中的方法进行实现
 * */
void Threadpool::Epolldata::process_pipe_data(int fd){
	char buffer[20];
	memset(buffer, '\0', 20);
	/*
	 *	对于这一块的代码, 必须进行加锁进行控制. <= 0的时候是--
	 *fd中没有数据可以读取, 返回错误情况,(非阻塞模式管道)
	 * */
	sem.Wait();
	if(read(fd, buffer, 20) <= 0){
		
		sem.Post();
		log.WriteFile(true, errno, "Threadpool::Epolldata::process_pipe_data(int fd) failure in read()");
	}
	sem.Post();
	int new_fd = atoi(buffer);
	sepoll.setnonblocking(new_fd);
	sepoll.addfd(new_fd);
}

/*
 *	对方将文件描述符已经关闭了
 * */
void Threadpool::Epolldata::close_fd(int fd){
	sepoll.delfd(fd);
	close(fd);
}

void Threadpool::Epolldata::process_data(int fd){
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	while(true){
		int recv_return = recv(fd, buffer, BUFF_SIZE, 0);
		if(recv_return == -1){
			/*
			 *	对于非阻塞模式下, 进行数据的读取
			 *	errno == EAGAIN || errno == EWOULDBLOCK
			 * */
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				send(fd, "ok\n", 3, 0);
				break;
			}
			/*
			 *	还欠一个检测一个心跳包
			 * */
			log.WriteFile(true, errno, "Threadpool::Epolldata::process_data recv failed ");
		}
		/*
		 *	对方已经关闭了, 上面已经处理了
		 * */
		else if(recv_return == 0){
						
		}
		/*
		 *	有数据的情况下
		 * */
		else {
			if(strncmp(buffer, std::string("end").c_str(), 3) == 0){
				send(fd, std::string("ok\n").c_str(), 3, 0);
				sepoll.delfd(fd);
				close(fd);
				break;
			}
			std::cout << "ing: " << buffer << std::endl;
		}
	}	
}

Threadpool::Epolldata::~Epolldata(){

}
