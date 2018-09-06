#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<pthread.h>

#define MAX_EVENT_NUM	1024
static int pipefd[2];

int setnonblocking(int fd){
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epoll_fd, int fd){
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

void sig_handler(int sig){
	int save_err = errno;
	int msg = sig;
	send(pipefd[1], (char*)&msg, 1, 0);
	errno = save_err;
}

void addsig(int sig){
	struct sigaction sa;
	memset(&sa, '\0', sizeof(struct sigaction));
	sa.sa_handler = sig_handler;
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}

int main(){
	const char *ip = "127.0.0.1";
	const int port = 7900;

	struct sockaddr_in server;
	bzero(&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &server.sin_addr);
	server.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	assert(sockfd != -1);

	int binder = bind(sockfd, (struct sockaddr*)&server, sizeof(server));
	assert(binder != -1);
	binder = listen(sockfd, 5);
	assert(binder != -1);

	struct epoll_event event[MAX_EVENT_NUM];
	int epoll_fd = epoll_create(5);
	assert(epoll_fd != -1);
	addfd(epoll_fd, sockfd);

	assert(socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd) != -1);
	setnonblocking(pipefd[1]);
	addfd(epoll_fd, pipefd[0]);

	addsig(SIGHUP);
	addsig(SIGCHLD);
	addsig(SIGTERM);
	addsig(SIGINT);

	while(true){
		int number = epoll_wait(epoll_fd, event, MAX_EVENT_NUM, -1);
		if(number < 0 && errno != EINTR){
			printf("epoll failure\n");
			break;
		}

		for(int i = 0; i < number; i++){
			int fd = event[i].data.fd;
			if(fd == sockfd){
				struct sockaddr_in client;
				socklen_t socklen = sizeof(client);
				int connfd = accept(sockfd, (struct sockaddr*)&client, &socklen);
				addfd(epoll_fd, connfd);
			}
			else if((fd == pipefd[0]) && (event[i].events & EPOLLIN)){
				int sig;
				char signals[1024];
				int ret = recv(pipefd[0], signals, sizeof(signals), 0);
				if(ret <= 0){
					continue;
				}
				else{
					for(int i = 0; i < ret; i++){
						switch(signals[i]){
							case SIGCHLD:
								printf("SIGCHLD is tirrger\n");
								break;
							case SIGHUP:
								printf("SIGHUP is trigger\n");
								break;
							case SIGTERM:
								printf("SIGTERM is trigger\n");
								break;
							case SIGINT:
								printf("SIGINT is trigger\n");
								break;
							default:
								printf("signal default\n");
								break;
						}
					}

				}
			}
			else{
				printf("otherthing handler\n");
			}
		}
	}


	printf("close fds\n");
	close(sockfd);
	close(pipefd[0]);
	close(pipefd[1]);
	return 0;
}
