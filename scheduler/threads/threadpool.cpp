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
	 *  初始化链接服务器的相关配置信息
	 * */
	Json json;
	json.ParseConfigure();
	for(int i = 0; i < thread_num; i++){
	    ser_info.push_back( json.GetSer_info(i));
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
	/*
	*	将其进行分开
	*/ 
	if(pthread_detach(pthread_self())){
		log.WriteFile(true, pthread_self() , "Threadpool::Threadpool failed in pthread_detach: ");
	}
	
	Threadpool *mythis = (Threadpool*)arg;
	Epdata epdata;
	epdata.sepoll.setnonblocking(mythis->pipe_read);
	epdata.sepoll.addfd(mythis->pipe_read);
	
	while(epdata.sub_flags || mythis->sum_flags){

		int epoll_num = epoll_wait(epdata.sepoll.getFD(), epdata.events, EVENT_NUM, -1);
		if(epoll_num == -1){
			if(errno == EINTR){
				continue;
			}
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
					//epdata.process_data(fd, mythis->ser_info);
					/*修改成阻塞模式下进行处理*/
					epdata.reply_http_info(fd, ser_info);
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
	//sepoll.setnonblocking(new_fd);
	sepoll.addfd(new_fd);
}

/*
 *	对方将文件描述符已经关闭了
 * */
void Threadpool::Epolldata::close_fd(int fd){
	sepoll.delfd(fd);
	close(fd);
}


/*
 *	单独测试其作为多线程来进行的测试, 不用服务器来做.
 * */
void Threadpool::Epolldata::process_data(int fd, std::vector<Sinfo> &ser){
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
			/*
			   if(strncmp(buffer, std::string("end").c_str(), 3) == 0){
			   send(fd, std::string("ok\n").c_str(), 3, 0);
			   sepoll.delfd(fd);
			   close(fd);
			   break;
			   }
			   std::cout << "ing: " << buffer << std::endl;
			   */
			   //reply_http(fd);
			   reply_http_info(fd, ser);
		}
	}	
}

Threadpool::Epolldata::~Epolldata(){

}

/*封装一个返回的报文, 并选用其中一个配置的服务器信息*/
void Threadpool::Epolldata::reply_http_info(int fd, std::vector<Sinfo> &ser){
	std::string reply = std::string(" http1.0 403 Forbidden");
	if(fd <= 0) {
		log.WriteFile(false, -1, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info() in failure arg");
		send(fd, reply.c_str(), reply.size(), 0);
		return ;
	}
	
	unsigned int num = ser.size();
	num = rand()  % num;
	
	int soc_cli=socket(PF_INET,SOCK_STREAM,0);
	/* creat server sockaddr_in */
	struct sockaddr_in ser_addr;
	ser_addr.sin_family=PF_INET;
	ser_addr.sin_addr.s_addr=inet_addr(ser[num].GetIP().c_str());
	ser_addr.sin_port=htons(ser[num].GetPort());//8888 port number has no ""
	
	if(connect(soc_cli,(struct sockaddr*)&ser_addr,sizeof(ser_addr))==-1){
		printf("connect error");
		log.WriteFile(false, -1, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info()::connect() in failure");
		send(fd, reply.c_str(), reply.size(), 0);
		return ;
	} 

	int pipefd[2] = {0, 0};
	int ret = pipe(pipefd);
	if(ret == -1){
		log.WriteFile(false, errno, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info()::pipe failure in failure");
	}

	ret = splice(fd, NULL, pipefd[1], NULL, BUFF_SIZE*BUFF_SIZE, SPLICE_F_MOVE | SPLICE_F_MORE);
	if(ret  == -1){
		log.WriteFile(false, errno, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info()::splice() in failure");
	}

	ret = splice(pipefd[0], NULL, soc_cli, NULL, BUFF_SIZE*BUFF_SIZE, SPLICE_F_MOVE | SPLICE_F_MORE);
	if(ret  == -1){
		log.WriteFile(false, errno, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info()::splice() in failure");
	}

	ret = splice(soc_cli, NULL, pipefd[1], NULL, BUFF_SIZE*BUFF_SIZE, SPLICE_F_MOVE | SPLICE_F_MORE);	
	if(ret  == -1){
		log.WriteFile(false, errno, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info()::splice() in failure");
	}

	ret = splice(pipefd[0], NULL, fd, NULL, BUFF_SIZE*BUFF_SIZE, SPLICE_F_MOVE | SPLICE_F_MORE);	
	if(ret  == -1){
		log.WriteFile(false, errno, "Threadpool::Epolldata::process_pipe_data(int fd, std::vector<Sinfo> &ser)::reply_http_info()::splice() in failure");
	}

	close(pipefd[0]);
	close(pipefd[1]);
	close(soc_cli);
}

/*定义一个测试的返回报文信息*/
void reply_http(int fd){
	if(fd <= 0){
		printf("reply_http failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}

	char buffer_sum[BUFF_SIZE*10];
	memset(buffer_sum, '\0', BUFF_SIZE*10);

	const char *buffer_status_lines = "HTTP/1.1 200 OK";
	const char *buffer_headers = "Content-Type: text/html;charset=ISO-8859-1";
	char buffer_file_size[BUFF_SIZE];
	char buffer_file[BUFF_SIZE];
	memset(buffer_file_size, '\0', BUFF_SIZE);
	memset(buffer_file, '\0', BUFF_SIZE);

	struct stat statbuff;
	memset(&statbuff, '\0', sizeof(statbuff));
	int stat_return = stat(FILE_PATH, &statbuff);
	if(stat_return == -1){
		printf("stat() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}
	sprintf(buffer_file_size, "%s", "Content-Length: ");
	sprintf(buffer_file_size + strlen(buffer_file_size), "%d", statbuff.st_size);

	strncpy(buffer_sum, buffer_status_lines, strlen(buffer_status_lines));
	strncpy(buffer_sum + strlen(buffer_sum), "\r\n", 2);
	strncpy(buffer_sum + strlen(buffer_sum), buffer_headers, strlen(buffer_headers));
	strncpy(buffer_sum + strlen(buffer_sum), "\r\n", 2);
	strncpy(buffer_sum + strlen(buffer_sum), buffer_file_size, strlen(buffer_file_size));
	strncpy(buffer_sum + strlen(buffer_sum), "\r\n", 2);
	strncpy(buffer_sum + strlen(buffer_sum), "\r\n", 2);
	int fd_read = open(FILE_PATH, O_RDONLY);
	if(fd_read <= 0){
		printf("open() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}
	read(fd_read, buffer_file, statbuff.st_size);
	strncpy(buffer_sum + strlen(buffer_sum), buffer_file, strlen(buffer_sum));
	send(fd, buffer_sum, strlen(buffer_sum), 0);
	close(fd_read);
}
