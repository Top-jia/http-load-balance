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
#include<string>
#include<map>

/*	请求错误:
 *		REQUEST_SUCCESS:		客户端请求文件成功
 *		REQUEST_NON_FOUND:		客户端请求的文件未找到
 *		PARSE_ERROR:			服务器解析客户端语法发生错误
 *		SERVER_ERROR:			服务器本身发生了错误(连接过多, 或其他情况)
 * */
enum{
	REQUEST_SUCCESS	= 0;
	REQUEST_NON_FOUND;
	PARSE_ERROR;
	SERVER_ERROR;
}REQUEST_STATE;

#define BUFF_SIZE	127

/*定义http_head的解析http请求报头*/
typedef struct http_head{
	std::string que_methon;
	std::string que_url;
	std::string que_protocol_version;
	/*这里用map主要是因为, 必须用其中的数据和名字来区分开, 并且为了更好的解析*/
	std::map<std::string, std::string> que_head_array;

	std::string que_data;
	bool status;
}Rhead;
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


/*	对于文本解析情况, 可以分情况(状态机)来讨论情况
 *		请求方法Get:只提供一种方法来解析
 *					200 OK
 *					404 资源不存在
 *		解析错误:	在解析中http请求报文的时候, 发生
 *		了错误(请求语法错误), 服务器将会给其发送,400 Not Fount
 *
 *		服务器出现状况: 或者崩溃了.
 *		出现解析http报的接口, 使用主从状态机.
 *			主状态机和从状态机配合
 *			其中:主状态机负责判断
 *					从缓存区读取一段('\r''\n')结尾的段.
 *				根据其中的状态, 来进行调整是否为请求行,
 *				还是头部字段, 并获取其中的信息, 放入固
 *				定的位置.
 * */
/*主状态机的状态情况: 正在分析请求行, 正在分析请求头部字段*/
enum CHECK_STATE{
	CHECK_STATE_REQUESTLINE = 0,
	CHECK_STATE_HEADER
};

/*从状态机的状态情况, 读取到一个完整的行, 读取不完整, 行错误*/
enum LINK_STATE{
	LINK_GOOD = 0,
	LINK_BAD,
	LINK_UNCOMPLETION
};

/*HTTP请求的就结果, 分为多个状态
 *	NO_REQUEST		读取的数据不够, 仍需从recv()中读取数据来分析
 *	GET_REQUEST		获取了一个完整的http请求
 *	BAD_REQUEST		客户端请求的语法错误
 *	INTERNAL_ERROR	服务器内部错误
 *	CLOSE_CONNECT	客户端已经关闭
 * */
enum HTTP_CODE{
	NO_REQUEST = 0,
	GET_REQUEST,
	BAD_REQUEST,
	INTERNAL_ERROR,
	CLOSE_CONNECT
};

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
			if(-1 == parse_text(buffer)){
			}
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
