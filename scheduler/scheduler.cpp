#include"scheduler.hpp"

extern Logger log;
extern Sem sem;
/*
 *	初始化相关调度器的ip和端口
 * */
Scheduler::Scheduler(){
	
	//log.WriteFile(false, -1, std::string("test info"));
	Json json;
	json.ParseConfigure();
	
	sche_info = json.GetSche_info();
	
	/*
	for(int i = 0; i < SER_NUM; i++){
		ser_info[i] = json.GetSer_info(i);
	}
	*/

	methon = json.GetMethon();
	memset(sepoll_events, '\0', MAX_EVENT_NUM * sizeof(struct epoll_event));

	int sockpair_return = socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair);
	if(sockpair_return != 0){
		log.WriteFile(true, errno, "Scheduler::Scheduler() socketpair failed");
	}
}

/*
 *	创建连接和相关结构
 * */
void Scheduler::CreateLink(){
	scheduler_fd = socket(AF_INET, SOCK_STREAM, 0); 
	if(scheduler_fd <= 0){
		log.WriteFile(true, errno, "Scheduler::Createlink_socket failed ");
	}

	struct sockaddr_in sche;
	memset(&sche, '\0', sizeof(sche));
	sche.sin_family = AF_INET;
	inet_pton(AF_INET, sche_info.GetIP().c_str(), &sche.sin_addr);
	sche.sin_port = htons(sche_info.GetPort());

	int binder = bind(scheduler_fd, (struct sockaddr*)&sche, sizeof(sche));
	assert(binder != -1);
	binder = listen(scheduler_fd, 5);//最大值为128???
	assert(binder != -1);
}

/*
 *	运行相关的程序
 *	将pipe_fd替换成sockpair
 *		第一步:
 *			其中sockpair[0]作为主线程往进写入fd, sockpair[1]为子线程从中读入fd,
 *		第二部: 对描述符的设置
 *			scheduler_fd 只负责接受连接, 并将其通过sockpair[0]送到子线程.
 *			其中epoll_wait监控的事件, 只有scheduler_fd, 其余的监控交给子线程去处理.
 * */

void Scheduler::Run(){
	 //		加入管道的描述符到主线程的epoll中
	 //并对其事件进行写入, (当有连接请求的时候)可写事件	 
	CreateLink();
	/*先设置scheduler_fd设置其事件为EPOLLIN(阻塞与否, 不重要)*/
	struct epoll_event event;
	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd = scheduler_fd;
	event.events = EPOLLIN;
	sepoll.addfd(scheduler_fd, &event);
	
	/*用于测试*/
	//char buff[127];
	//std::cout << getcwd(buff, 127) << std::endl;
	//log.WriteFile(false, -1, std::string("test info"));
	/*对双端管道设置非阻塞模式*/
	sepoll.setnonblocking(sockpair[1]);
	
	Threadpool pool(SER_NUM, sockpair[1]);
	while(true){
		int epoll_return = epoll_wait(sepoll.getFD(), sepoll_events, MAX_EVENT_NUM, -1);
		if(epoll_return == -1){
			if(errno == EINTR){
				continue;
			}
			log.WriteFile(true, errno, "Scheduler::Run epoll_wait failed");
		}
		else if(epoll_return == 0){
			log.WriteFile(false, 0, "Scheduler::Run epoll_Wait failed epoll_return = 0");
			continue;
		}
		else{
			for(int i = 0; i < epoll_return; i++){
				int fd = sepoll_events[i].data.fd;
				/*有连接事件到来*/
				if(fd == scheduler_fd){
					accept_link(fd);
					continue;
				}
			}	
		}	
	}
}

/*
 *	增加写入文件描述符的数量.
 * */
void Scheduler::addBufferLen(char *buffer){
	for(int i = 0; i < 10; i++){
		if(buffer[i] == '\0'){
			buffer[i] = '*';
		}
	}
}

/*
 *	创建连接, 并写入管道中.
 * */
void Scheduler::accept_link(int fd){
	if(fd <= 0){
		log.WriteFile(true, errno, "Scheduler::accept_link failure in args ");
	}
	char buffer_fd[BUFF_SIZE];
	memset(buffer_fd, '\0', BUFF_SIZE);
	struct sockaddr_in client;
	socklen_t cli_len = sizeof(client);
	memset(&client, '\0', cli_len);
	
	int accept_fd = accept(fd, (struct sockaddr*)&client, &cli_len);
	if(accept_fd == -1){
		log.WriteFile(true, errno, "Scheduler::accept return failed ");
	}
	sprintf(buffer_fd, "%d", accept_fd);
	addBufferLen(buffer_fd);
	write(sockpair[0], buffer_fd, 10);
	//write(sockpair[0], buffer_fd, strlen(buffer_fd));
}
/*
void Scheduler::Run(){
	while(true){
		int epoll_event_num = epoll_wait(sepoll.getFD(), sepoll_events, MAX_EVENT_NUM, -1);
		if(epoll_event_num == -1){
			log.WriteFile(true, errno, "Scheduler::Run_epoll_wait failed ");
		}
		
		// 	这个对于超时的处理timeout
		 
		else if(epoll_event_num == 0){
			log.WriteFile(false, 0, "Scheduler::Run timeout");
			continue;
		}

		char msg_buff[BUFF_SIZE];
		for(int i = 0; i < epoll_event_num; i++){
			FD args_fd = sepoll_events[i].data.fd;
			
			//  有连接的事件到来
			 
			if(scheduler_fd == args_fd){
				struct sockaddr_in new_connect_addr;
				socklen_t addr_len = sizeof(new_connect_addr);
				int new_connect_fd = accept(args_fd, (struct sockaddr*)&new_connect_addr,\
					   	&addr_len);
				if(new_connect_fd == -1){
					log.WriteFile(true, errno, "Scheduler::Run_epoll_wait accept failed");
				}

				sepoll.setnonblocking(new_connect_fd);
				sepoll.addfd(new_connect_fd);
			}
			 
			 //	对关闭数据fd进行操作
			 
			else if(sepoll_events[i].events & EPOLLRDHUP){
				sepoll.delfd(args_fd);
				close(args_fd);
			}
			
			 //	对发过来的数据进行接受
			 
			else if(sepoll_events[i].events & EPOLLIN){

				while(true){
				
					memset(msg_buff, '\0', BUFF_SIZE * sizeof(char));
					int recv_return = recv(args_fd, msg_buff, BUFF_SIZE-1, 0);
					if(recv_return == -1){
						
						 //	对于非阻塞的模式下, 进行数据的读取.
						 //		errno == EAGIN || errno == EWOULDBLOCK表示数据已经读取完成
						
						if((errno == EAGAIN) || errno == EWOULDBLOCK){
							send(args_fd, std::string("ok\n").c_str(), 3, 0);
							break;
						}
						// 此外的情况下: 表示还有其他情况要进行处理 
						log.WriteFile(true, errno, "Scheduler::Run_epoll_wait recv failed");
					}
					
					 //	对方关闭了, 因为已经操作过, 则不进行
					 
					else if(recv_return == 0){
						
					}
					
					// 有数据情况下的处理
					 
					else{
							if(strncmp(msg_buff, std::string("end").c_str(), 3) == 0){
								send(args_fd, std::string("ok\n").c_str(), 3, 0);
								sepoll.delfd(args_fd);
								close(args_fd);
								break;
							}
							std::cout << "ing: " << msg_buff << std::endl;
					}
				}
			}
			else {
				std::cout << " something else happened " << std::endl;
			}
		}		
	}
}

 * void Scheduler::Run() : 0{
	
	struct sockaddr_in scheduler_cli;
	socklen_t cli_len = sizeof(scheduler_cli);
	memset(&scheduler_cli, '\0', sizeof(scheduler_cli));
	FD connfd = accept(scheduler_fd, (struct sockaddr*)&scheduler_cli, &cli_len);
	if(connfd <= 0){
		log.WriteFile(false, errno, "Scheduler::Run_accept failed ");
	}
	
	char buffer[BUFF_SIZE];
	memset(buffer, '\0', BUFF_SIZE);
	while(true){
		int recv_num = recv(connfd, buffer, BUFF_SIZE, 0);
		if(recv_num == -1){
			log.WriteFile(false, errno, "Scheduler::Run_recv failed errno = ");
		}

		if(recv_num == 0 || strncmp(std::string("end").c_str(), buffer, 3) == 0){
			close(connfd);
			break;
		}

		std::cout << buffer;
		memset(buffer, '\0', strlen(buffer));
		send(connfd, std::string("ok\n").c_str(), strlen("ok\n"), 0);
	}
	close(scheduler_fd);
}
*/
