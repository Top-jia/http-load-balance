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
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>

#define BUFF_SIZE	127
#define MAX_EVENT	1024
pthread_mutex_t mutex;

/*创建socket协议族, 并绑定ip和端*/
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

/*分装epoll事件类*/
class Epoll{
	int epoll_fd;
	std::stack<int> fd_stack;
	public:
	/*对描述符设置非阻塞模式*/
	static void setnonblocking(int fd){
		if(fd <= 0){
			std::cout << "Epoll::setnonblockking(int fd) args failed errno = "\
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
			std::cout << "Epoll::Epoll(int pipefd) epoll_create epoll_fd == -1 failure errno = " \
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
			std::cout << "Epoll::addfd(int fd, struct epoll_event *event = NULL) args failed errno = "\
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
			std::cout << "Epoll::delfd(int fd) args failed errno = " \
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
	int *sockpair;
	bool sum_flags;
	struct epoll_event events[MAX_EVENT];

	static Ddata *m_instance;
	/*构造函数传递相关的数据结构*/
	Ddata(int _sockfd, int sockpair_0, int sockpair_1, Epoll &_edata):\
		sockfd(_sockfd), edata(_edata), sum_flags(true){
			sockpair = new int[2];
			sockpair[0] = sockpair_0;
			sockpair[1] = sockpair_1;
			memset(events, '0', sizeof(struct epoll_event) * MAX_EVENT);
		}
	public:
	/*生产单个类的方法*/
	static Ddata* get_instance(int _sockfd, int pipe_read, int pipe_write, Epoll &_edata){
		if(m_instance == NULL){
			if(m_instance == NULL){
				pthread_mutex_lock(&mutex);
				m_instance = new Ddata(_sockfd, pipe_read, pipe_write, _edata);
				pthread_mutex_unlock(&mutex);
			}
		}
		return m_instance;
	}
	/*释放相关资源*/
	~Ddata(){
		delete []sockpair;
	}
	/*运行epoll_wait中主程序*/
	void run(){
		char write_buffer[BUFF_SIZE];
		memset(write_buffer, '\0', BUFF_SIZE);
		while(sum_flags){
			int epoll_num = epoll_wait(edata.getfd(), events, MAX_EVENT, -1);
			if(epoll_num == -1){
				std::cout << "Ddata::run() epoll_wait failure epoll_num == -1 errno = "\
				<< errno <<"strerror = " << strerror(errno) << std::endl;
				exit(EXIT_FAILURE);
			}
			else if(epoll_num == 0){
				std::cout << "Ddata::run() epoll_wait failure epoll_num ==  0 errno = "\
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
					/*管道sockpair[0]中有读事件的发生*/
					if(sockpair[0] == fd && events[i].events & EPOLLIN){
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
						strcpy(write_buffer, "sockpair[0] EPOLLOUT");
						deal_data(fd);
						continue;
					}

					/*管道sockpair[0]中写的事件被处理*/
					if(sockpair[0] == fd && events[i].events & EPOLLOUT && strlen(write_buffer) != 0){
						std::cout << "sockpair[0] EPOLLOUT" << write_buffer << std::endl;
						memset(write_buffer, '\0', strlen(write_buffer));
						continue;
					}
				}
			}
		}
	}
	/*客户端来连接服务器, 同时加入描述符到管道的写端*/
	void link_server(int fd){
		if(fd <= 0){
			std::cout << "Ddata::link_server(int fd) failure fd <= 0 errno = " \
			<< errno <<"strerror = " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		struct sockaddr_in client;
		socklen_t cli_len = sizeof(client);
		memset(&client, '\0', cli_len);
		int accept_fd = accept(fd, (struct sockaddr*)&client, &cli_len);
		if(accept_fd == -1){
			std::cout << "Data::link_server(int fd) failure accept_fd errno = " \
				<< errno <<"strerror = " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}

		char fd_buffer[BUFF_SIZE];
		memset(fd_buffer, '\0', BUFF_SIZE);
		sprintf(fd_buffer, "%d", accept_fd);
		write(sockpair[1], fd_buffer, strlen(fd_buffer));
	}

	/*管道中有读事件的发生, 并且将读出来的文件描述符加入epoll事件集中*/
	void join_epollevent(int fd){
		if(fd <= 0){
			std::cout << "Ddata::join_epollevent(int fd) failure in fd <= 0 errno = " \
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
			std::cout << "Ddata::deal_closefd(int fd) failure in fd <= 0 errno = "\
				<< errno <<"strerror = " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		edata.delfd(fd);
		close(fd);
	}
	/*处理客户端的数据*/
	void deal_data(int fd){
		if(fd <= 0){
			std::cout << "Ddata::deal_data(int fd) failure in fd <= 0 errno = " \
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
				std::cout << "Edata::deal_data(int fd) failure in recv_return == -1 errno = " \
				<< errno <<"strerror = " << strerror(errno) << std::endl;
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
	int sockpair[2];
	
	/*创建双端管道*/
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockpair) == -1){
		printf("socketpair create failure errno = %d str_error = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*将双端管道设置都设置为非阻塞模式*/
	Epoll::setnonblocking(sockpair[0]);
	Epoll::setnonblocking(sockpair[1]);
	Epoll edata;

	struct epoll_event event;
	memset(&event, '\0', sizeof(struct epoll_event));
	/*对socket的fd注册epoll事件*/
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLRDHUP;
	edata.addfd(sockfd, &event);

	/*		对sockpair[0]或[1]注册相同的事件, 读数据的时候, 是非阻塞模式下，并对数据只触发一次
	 *	. 对此, 可以由发送负载子线程池情况.
	 * */
	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd = sockpair[0];
	event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP; 
	edata.addfd(sockpair[0], &event);
	
	event.data.fd = sockpair[1];
	edata.addfd(sockpair[1], &event);

	pthread_mutex_init(&mutex, NULL);	
	Ddata *ddata = Ddata::get_instance(sockfd, sockpair[0], sockpair[1], edata);
	ddata->run();
	
	pthread_mutex_destroy(&mutex);
	return 0;
}
