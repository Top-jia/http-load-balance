#include<iostream>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<string>
#include<stack>

#define BUFF_SIZE	127
#define MAX_EVENT_NUM	1024

/*创建socket协议族*/
int create_socket(){
	const char *ip = "127.0.0.1";
	const int port = 7900;
	
	struct sockaddr_in server;
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &server.sin_addr);
	server.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		fprintf(stderr, "create_socket()--socket() failure errno = %d str_err = %s", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	/*设置地址重用*/
	int reuse = -1;
	int err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if(err == -1){
		fprintf(stderr, "create_socket()--setsockopt() failure errno = %d str_err = %s", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int binder = bind(sockfd, (struct sockaddr*)&server, sizeof(server));
	if(binder == -1){
		fprintf(stderr, "create_socket()--bind() failure errno = %d str_err = %s", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	binder = listen(sockfd, 5);
	if(binder == -1){
		fprintf(stderr, "create_socket()--listen() failure errno = %d str_err = %s", errno, strerror(errno));
	}

	return sockfd;
}

/*封装epoll类*/
class Epoll{
		int epoll_fd;
		int sockfd;
		std::stack<int>	stack_fd;

	public:
		/*创建Epoll文件描述符*/
		Epoll(int _sockfd):sockfd(_sockfd){
			if(sockfd <= 0){
				fprintf(stderr, "Epoll()_Epoll() args failure");
				exit(EXIT_FAILURE);
			}
			
			epoll_fd = epoll_create(5);
			if(epoll_fd == -1){
				fprintf(stderr, "Epoll()_Epoll()--epoll_create() failure errno = %d str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		
		/*对描述符设置非阻塞模式*/
		void setnonblocking(int fd){
			if(fd <= 0){
				fprintf(stderr, "Epoll()__setnonblocking() args failure");
				exit(EXIT_FAILURE);
			}
			int old_option = fcntl(fd, F_GETFD);
			int new_option = old_option | O_NONBLOCK;
			fcntl(fd, F_SETFL, new_option);
		}

		/*增加文件描述符*/
		void addfd(int fd, struct epoll_event *event = NULL){
			if(fd <= 0){
				fprintf(stderr, "Epoll()__addfd() args failure");
				exit(EXIT_FAILURE);
			}
			
			if(event != NULL){
				int err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
				if(err == -1){
					fprintf(stderr, "Epoll()__addfd()---epoll_ctl() errno = %d str_err = %s", errno, strerror(errno));
					exit(EXIT_FAILURE);
				}
				return ;
			}

			struct epoll_event events;
			bzero(&events, sizeof(struct epoll_event));
			events.data.fd = fd;
			events.events = EPOLLIN | EPOLLRDHUP;
			int err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &events);
			if(err == -1){
				fprintf(stderr, "Epoll()__addfd()---epoll_ctl() errno = %d str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			stack_fd.push(fd);
		}

		/*修改文件描述符中的事件*/
		void modfd(int fd, struct epoll_event *event){
			if(event == NULL || fd <= 0){
				fprintf(stderr, "Epoll()__modfd() args failure");
				exit(EXIT_FAILURE);
			}
			
			int err = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event);
			if(err == -1){
				fprintf(stderr, "Epoll()__modfd()---epoll_ctl() err = %d str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		/*删除文件描述符*/
		void delfd(int fd){
			if(fd <= 0){
				fprintf(stderr, "Epoll()__delfd() args failure");
				exit(EXIT_FAILURE);
			}

			int err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
			if(err == -1){
				fprintf(stderr, "Epoll()__delfd()--epoll_ctl() failure error = %d str_err = %s ", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

		/*销毁栈中的数据, 并且将已经存在的fd描述符进行销毁*/
		~Epoll(){
			while(stack_fd.empty()){
				int new_fd = stack_fd.top();
				stack_fd.pop();
				if(new_fd != sockfd){
					close(new_fd);
				}
			}
		}

		/*得到epoll中的内核事件集*/
		int getfd(){
			return epoll_fd;
		}

		/*获得栈的描述符集合*/
		std::stack<int> getfd_set(){
			return stack_fd;
		}

		/*设置写事件*/
		void set_write(int fd){
			struct epoll_event events;
			bzero(&events, sizeof(struct epoll_event));
			events.data.fd = fd;
			/*
			events.events = fcntl(fd, F_GETFL);
			events.events |= EPOLLOUT;
			events.events |= ~EPOLLIN;
			*/
			events.events = EPOLLOUT | EPOLLRDHUP;
			modfd(fd, &events);
		}

		/*设置读事件*/
		void set_read(int fd){
			struct epoll_event events;
			bzero(&events, sizeof(struct epoll_event));
			events.data.fd = fd;
			/*
			events.events = fcntl(fd, F_GETFL);
			events.events |= EPOLLIN;
			events.events |= ~EPOLLOUT;
			*/
			events.events = EPOLLIN | EPOLLRDHUP;
			modfd(fd, &events);
		}
};

/*定义一个简单的结构来对数据处理进行发送*/
struct d_hander{
	std::string str;
	int notfd;
};

/*封装数据发送的类, 目地是当一个用户发送数据到来时候, (通过不同的事件集的设置)转发给其他的用户*/
class Ddata{
		Epoll epoll_info;
		struct epoll_event events[MAX_EVENT_NUM];
		bool sum_flags;
		d_hander dhander;
		int sockfd;
	public:
		/*构造数据的发送和接受类*/
		Ddata(int _sockfd):epoll_info(_sockfd), sum_flags(true), sockfd(_sockfd){
			if(sockfd <= 0){
				fprintf(stderr, "Ddata::Ddata() args failure");
				exit(EXIT_FAILURE);
			}
			dhander.notfd = -1;
			bzero(events, sizeof(struct epoll_event) * MAX_EVENT_NUM);

			epoll_info.setnonblocking(sockfd);
			epoll_info.addfd(sockfd);
		}

		/*运行整个程序*/
		void run(){
			while(sum_flags){
				int reply_num = epoll_wait(epoll_info.getfd(), events, MAX_EVENT_NUM, -1);
				if(reply_num == -1){
					fprintf(stderr, "Ddata::run() epoll_wait() failure errno = %d str_err = %s", errno, strerror(errno));
					sum_flags = false; 
					break;
				}
				else if(reply_num == 0){
					printf("this way isnot happend\n");
					continue;
				}
				else{
					for(int i = 0; i < reply_num; i++){
						int fd = events[i].data.fd;
						/*有连接的事件到来*/
						if(fd == sockfd){
							accept_fd();
							continue;
						}
						/*有关闭的事件到来*/
						if(events[i].events & EPOLLRDHUP){
							close_fd(fd);
							continue;
						}	
						/*有写事件到来*/
						if(events[i].events & EPOLLOUT){
							handle_write(fd);
							continue;		
						}
						/*有读事件到来*/
						if(events[i].events & EPOLLIN){
							handle_read(fd);																		
						}
					}
				}
			}
		}
	/*接受客户端连接*/
	void accept_fd(){
		struct sockaddr_in client;
		socklen_t client_len = sizeof(struct sockaddr_in);
		bzero(&client, client_len);
		int fd = accept(sockfd, (struct sockaddr*)&client, &client_len);
		if(fd == -1){
			fprintf(stderr, "Ddata::accept_fd() failure errno = %d str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		epoll_info.setnonblocking(fd);
		epoll_info.addfd(fd);
	}

	/*有关闭事件的到来*/
	void close_fd(int fd){
		if(fd <= 0){
			fprintf(stderr, "Ddata::close_fd() args failure");
			exit(EXIT_FAILURE);
		}
		
		epoll_info.delfd(fd);
		close(fd);
	}
	
	/*处理写的事件*/
	void handle_write(int fd){
		if(fd <= 0){
			fprintf(stderr, "Ddata::close_fd() args failure");
			exit(EXIT_FAILURE);
		}
		send(fd, (void *)dhander.str.c_str(), dhander.str.size(), 0);
		epoll_info.set_read(fd);
	}

	/*处理读的事件*/
	void handle_read(int fd){
		if(fd <= 0){
			fprintf(stderr, "Ddata:;close_fd() args failure");
			exit(EXIT_FAILURE);
		}
		char buffer[BUFF_SIZE];
		bzero(buffer, sizeof(char) * BUFF_SIZE);
		
		/*读取数据*/
		while(true){
			int data_num = recv(fd, buffer, BUFF_SIZE, 0);
			if(data_num == -1){
				if(errno == EAGAIN || errno == EWOULDBLOCK){
					break;
				}

				fprintf(stderr, "Ddata::handle_read()---recv() errno = %d str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			/*对方关闭, 则一般不会发生*/
			else if(data_num == 0){
			}
			else{
				dhander.notfd = fd;
				dhander.str	= buffer;		
				bzero(buffer, strlen(buffer));
				std::stack<int> st = epoll_info.getfd_set();
				while(st.size()){
					int new_fd = st.top();
					st.pop();
					if(new_fd == sockfd || new_fd == fd){
						continue;
					}
					epoll_info.set_write(new_fd);
				}
			}	
		}
	}
};

int main(){
	int fd = create_socket();
	Ddata d_data(fd);
	d_data.run();
	close(fd);
	return 0;
}
