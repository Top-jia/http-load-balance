#include"../threadpool.hpp"
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<assert.h>
#include<iostream>
#include<sys/epoll.h>
#include<stack>

#define BUFF_SIZE	127
#define MAX_EVENT	1024
/*
 *		创建socket协议族, 并绑定ip和端口
 * */
int create_socket(){
	const char *ip = "127.0.0.1";
	int port = 7800;
	struct sockaddr_in server;
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &server.sin_addr);
	server.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	int binder = bind(sockfd, (struct sockaddr*)&server, sizeof(server));
	assert(binder != -1);
	binder = listen(sockfd, 5);
	return sockfd;
}

/*
 *	封装Epoll类,
 * */
class Epoll{
		int epoll_fd;
		std::stack<int> fd_stack;
	public:
		/*对描述符设置非阻塞模式*/
		static void setnonblocking(int fd){
			if(fd <= 0){
				std::cout << "Epoll::setnonblockking(int fd) args failed " << std::endl;
				exit(EXIT_FAILURE);
			}
			int old_option = fcntl(fd, F_GETFL);
			int new_option = old_option | O_NONBLOCK;
			fcntl(fd, F_SETFL, new_option);
		}
		/*对于初始化的Epoll的构造函数*/
		Epoll(){
			epoll_fd = epoll_create(5);
			if(epoll_fd == -1){
				std::cout << "Epoll::Epoll(int pipefd) epoll_create epoll_fd == -1 failure " << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		/*释放epoll中的相关资源*/
		~Epoll(){
			while(!fd_stack.empty()){
				int fd = fd_stack.top();
				delfd(fd);
				fd_stack.pop();
				close(fd);
			}
			close(epoll_fd);
		}
		/*给epoll中添加文件描述符*/
		void addfd(int fd, struct epoll_event *event = NULL){
			if(fd <= 0){
				std::cout << "Epoll::addfd(int fd, struct epoll_event *event = NULL) args failed " << std::endl;
				exit(EXIT_FAILURE);
			}
			if(event != NULL){
				event->data.fd = fd;
				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
				return ;
			}
			struct epoll_event events;
			memset(&events, '\0', sizeof(struct epoll_event));
			events.data.fd = fd;
			events.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
			epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &events);
			fd_stack.push(fd);
		}

		/*删除epoll中的文件描述符*/
		void delfd(int fd){
			if(fd <= 0){
				std::cout << "Epoll::delfd(int fd) args failed " << std::endl;
				exit(EXIT_FAILURE);
			}
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		}

		/*修改epoll中的文件描述符的注册的事件*/
		void modfd(int fd, struct epoll_event *event){
			if(fd > 0 && event != NULL){
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event);	
			}
		}
		
		/*得到epoll的文件描述符*/
		int getfd(){
			return epoll_fd;
		}
};

/*deal with data 处理数据的类*/
class Ddata{
		Epoll &edata;
		int sockfd;
		int *pipe_fd;
		bool sum_flags;
		struct epoll_event events[MAX_EVENT];

		static Ddata *m_instance;
		/*构造函数传递相关的数据结构*/
		Ddata(int _sockfd, int pipe_read, int pipe_write, Epoll &_edata):\
			sockfd(_sockfd), edata(_edata), sum_flags(true){
				pipe_fd = new int[2];
				pipe_fd[0] = pipe_read;
				pipe_fd[1] = pipe_write;
				memset(events, '0', sizeof(struct epoll_event) * MAX_EVENT);
			}
	public:
		/*生产单个类的方法*/
		static Ddata* get_instance(int _sockfd, int pipe_read, int pipe_write, Epoll &_edata){
			if(m_instance == NULL){
				m_instance = new Ddata(_sockfd, pipe_read, pipe_write, _edata);
			}
			return m_instance;
		}
		/*释放相关资源*/
		~Ddata(){
			delete []pipe_fd;
		}
		/*运行epoll_wait中主程序*/
		void run(){
			while(sum_flags){
				int epoll_num = epoll_wait(edata.getfd(), events, MAX_EVENT, -1);
				if(epoll_num == -1){
					std::cout << "Ddata::run() epoll_wait failure epoll_num == -1";
					exit(EXIT_FAILURE);
				}
				else if(epoll_num == 0){
					std::cout << "Ddata::run() epoll_wait failure epoll_num ==  0";
					exit(EXIT_FAILURE);
				}
				else{
					for(int i = 0; i < epoll_num; i++){
						int fd = events[i].data.fd;
						/*有客户端来连接服务器*/
						if(sockfd == fd){
							link_server(fd);
							continue;
						}
						/*管道中有读事件的发生*/
						if(pipe_fd[0] == fd && events[i].events & EPOLLIN){
							join_epollevent(fd);
							continue;
						}
						/*客户端关闭了*/
						if(events[i].events & EPOLLRDHUP){
							deal_closefd(fd);
							continue;
						}
						/*处理客户端数据*/
						if(events[i].events & EPOLLIN){
							deal_data(fd);
						}
					}
				}
			}
		}
		/*客户端来连接服务器, 同时加入描述符到管道的写端*/
		void link_server(int fd){
			if(fd <= 0){
				std::cout << "Ddata::link_server(int fd) failure fd <= 0 " << std::endl;
				exit(EXIT_FAILURE);
			}
			struct sockaddr_in client;
			socklen_t cli_len = sizeof(client);
			memset(&client, '\0', cli_len);
			int accept_fd = accept(fd, (struct sockaddr*)&client, &cli_len);
			if(accept_fd == -1){
				std::cout << "Data::link_server(int fd) failure accept_fd " << std::endl;
				exit(EXIT_FAILURE);
			}
			
			char fd_buffer[BUFF_SIZE];
			memset(fd_buffer, '\0', BUFF_SIZE);
			sprintf(fd_buffer, "%d", accept_fd);
			write(pipe_fd[1], fd_buffer, strlen(fd_buffer));
		}
		
		/*管道中有读事件的发生, 并且将读出来的文件描述符加入epoll事件集中*/
		void join_epollevent(int fd){
			if(fd <= 0){
				std::cout << "Ddata::join_epollevent(int fd) failure in fd <= 0 " << std::endl;
				exit(EXIT_FAILURE);
			}
			char fd_buffer[BUFF_SIZE];
			memset(fd_buffer, '\0', BUFF_SIZE);
			read(fd, fd_buffer, BUFF_SIZE);
			int accept_fd = atoi(fd_buffer);
			Epoll::setnonblocking(accept_fd);
			edata.addfd(accept_fd);
		}
		/*处理客户端关闭的文件描述符*/
		void deal_closefd(int fd){
			if(fd <= 0){
				std::cout << "Ddata::deal_closefd(int fd) failure in fd <= 0 " << std::endl;
				exit(EXIT_FAILURE);
			}
			edata.delfd(fd);
			close(fd);
		}
		/*处理客户端的数据*/
		void deal_data(int fd){
			if(fd <= 0){
				std::cout << "Ddata::deal_data(int fd) failure in fd <= 0 " << std::endl;
				exit(EXIT_FAILURE);
			}
			char msg_buffer[BUFF_SIZE];
			memset(msg_buffer, '\0', BUFF_SIZE);
			while(true){
				int recv_return = recv(fd, msg_buffer, BUFF_SIZE, 0);
				if(recv_return == -1){
					/*说明数据已经读取完成, 产生错误errno == EAGAIN || errno == EWOULDBLOCK*/
					if(errno == EAGAIN || errno == EWOULDBLOCK){
						break;
					}
					std::cout << "Edata::deal_data(int fd) failure in recv_return == -1 " << std::endl;
					exit(EXIT_FAILURE);
				}
				if(strncmp(msg_buffer, "all end", 7) == 0){
					sum_flags = false;
					break;
				}
				if(strncmp(msg_buffer, "end", 3) == 0){
					edata.delfd(fd);
					close(fd);
					break;
				}
				std::cout << "client_fd = " << fd << " recv_buffer = " << msg_buffer;
				send(fd, "ok\n", 3, 0);
				memset(msg_buffer, '\0', BUFF_SIZE);
			}
		}
};

Ddata* Ddata ::m_instance = NULL;
int main(){
	int sockfd = create_socket();
	int pipe_fd[2];
/*	
 *	pipe_fd[0]------为读端		非阻塞模式下读取
 *	pipe_fd[1]------为写端
 * */
	assert(pipe(pipe_fd) != -1);
	Epoll::setnonblocking(pipe_fd[0]);
	Epoll edata;
	
	/*对于管道的读端来说, 我们应该给其注册为(非阻塞模式下) 读事件|挂起事件|ET模式*/
	struct epoll_event event;
	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd  = pipe_fd[0];
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	edata.addfd(pipe_fd[0], &event);

	/*	对于管道的写端来说, 我们应该给其注册为 写事件|挂起事件, 不能设置EPLLET模式
	 *因为: ET将至触发一次, 不会再进行触发, (与我们所要的条件不符), 并且这样会出现
	 *epoll_wait每次多不会进行阻塞住, 每次都会有至少一个文件描述符返回, 即注册的
	 *可写事件的描述符.
	 **/
/*	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd = pipe_fd[1];
	event.events = EPOLLOUT | EPOLLRDHUP;
	edata.add(pipe_fd[1], &event);
*/
	
	/*再往epoll中注册sockfd描述符, 注册的事件为 读事件|关闭挂起的事件*/
	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLRDHUP;
	edata.addfd(sockfd, &event);
	
	Ddata *ddata = Ddata::get_instance(sockfd, pipe_fd[0], pipe_fd[1], edata);	
	ddata->run();

	Ddata *pdata = Ddata::get_instance(sockfd, pipe_fd[0], pipe_fd[1], edata);	
	if(ddata == pdata){
		std::cout << "ddata == pdata " << std::endl;
	}
	
	close(pipe_fd[1]);
	return 0;
}
