#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/stat.h>

#define BUFF_SIZE	127
/*创建socket协议族, 并绑定ip和端*/
int create_socket(){
	const char *ip = "127.0.0.1";
	int port = 7900;
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

/*设置非阻塞模式*/
void setnonblocking(int fd){
	if(fd <= 0){
		printf("accept() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
}

/*解析一个报文*/
int parse_http(int fd){
	if(fd <= 0){
		printf("accept() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}
	char buffer[BUFF_SIZE];
	while(true){
		memset(buffer, '\0', BUFF_SIZE);
		int recv_num = recv(fd, buffer, BUFF_SIZE, 0);
		if(recv_num == -1){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				break;
			}
			printf("recv failure errno = %d, str_err = %s\n", errno, strerror(errno));
			exit(0);
		}
		else if(recv_num == 0){
			printf("peer close\n");
			return -1;
		}
		else{
			printf("%s", buffer);
			fflush(stdin);
			fflush(stdout);
		}
	}
}

/*分装一个报文发送个客户端*/
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
	int stat_return = stat("index.html", &statbuff);
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
	int fd_read = open("index.html", O_RDONLY);
	if(fd_read <= 0){
		printf("open() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}
	read(fd_read, buffer_file, statbuff.st_size);
	strncpy(buffer_sum + strlen(buffer_sum), buffer_file, strlen(buffer_sum));
	send(fd, buffer_sum, strlen(buffer_sum), 0);
	close(fd_read);
}

int main(){
	int sockfd = create_socket();

	struct sockaddr_in client;
	socklen_t cli_len = sizeof(client);
	memset(&client, '\0', cli_len);
	int accept_fd = accept(sockfd, (struct sockaddr*)&client, &cli_len);
	setnonblocking(accept_fd);
	if(accept_fd == -1){
		printf("accept() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(0);
	}

	while(true){
		if(-1 == parse_http(accept_fd)){
			break;
		}
		reply_http(accept_fd);
	}
	close(accept_fd);
	close(sockfd);
	return 0;
}
