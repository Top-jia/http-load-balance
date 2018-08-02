#include"threadpool.hpp"
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<iostream>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<assert.h>
#include<string.h>
#include<sys/epoll.h>
#include<arpa/inet.h>

#define  BUFF_SIZE	127
#define	 MAX_EVENT	1024

int create_socket(){
	const char *ip = "127.0.0.1";
	int  port= 7800;
	struct sockaddr_in server;
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &server.sin_addr);
	server.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	int binder = bind(sockfd, (struct sockaddr*)&server, sizeof(server));
	assert(binder != -1);
	listen(sockfd, 5);
	return sockfd;
}

int main(){
	int pipe_fd[2];
	int sockfd = create_socket();
	int epoll_fd = epoll_create(5);
	assert(pipe2(pipe_fd, O_NONBLOCK) != -1);

	struct epoll_event event;
	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event);

	memset(&event, '\0', sizeof(struct epoll_event));
	event.data.fd = pipe_fd[1];
	event.events = EPOLLOUT | EPOLLET | EPOLLRDHUP;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fd[1], &event);

	char fd_buffer[BUFF_SIZE/10];
	memset(fd_buffer, '\0', BUFF_SIZE/10);
	struct epoll_event events[MAX_EVENT];
	Threadpool pool(3, pipe_fd[0]);
	while(1){
		int epoll_num = epoll_wait(epoll_fd, events, MAX_EVENT, -1);
		if(epoll_num == -1){
			printf("epoll_wait failed \n");
			exit(EXIT_FAILURE);
		}
		else if(epoll_num == 0){
			printf("epoll_num == 0 failure");
			continue;
		}
		else{
			for(int i = 0; i < epoll_num; i++){
				int fd = events[i].data.fd;
				if(fd == sockfd){
					struct sockaddr_in client;
					socklen_t cli_len = sizeof(struct sockaddr_in);
					memset(&client, '\0', cli_len);
					int new_fd = accept(fd, (struct sockaddr*)&client, &cli_len);
					assert(new_fd != -1);
					sprintf(fd_buffer, "%s", new_fd);
				}

				if(events[i].events & EPOLLOUT && strlen(fd_buffer) != 0){
					write(pipe_fd[1], fd_buffer, strlen(fd_buffer));
					memset(fd_buffer, '\0', strlen(fd_buffer));
				}
			
			}
		
		}
	
	}
	return 0;
}
