#include<iostream>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>

#define BUFF_SIZE	1024
/*
 *	主状态机可能两种状态, 请求当前行, 正在分析头部字段
 * */
enum CHECK_STATE{
	CHECK_STATE_REQUESTLINE = 0,
	CHECK_STATE_HEADER
};

/*
 *	从状态机, 行的读取, 读取一个完整的行, 行出错, 行的数据不完整
 * */
enum LINE_STATUS{
	LINK_OK = 1,
	LINK_BAD,
	LINK_OPEN
};

/*
 *	服务器处理, HTTP请求的结果: 
 *		NO_REQUEST	请求不完整
 *		GET_REQUEST	获取一个完整的客户请求
 *		BAD_REQUEST	客户请求语法错误
 *		FORBIDDEN_REQUEST	没有权限访问
 *		INTERNAL_ERROR		服务器内部错误
 *		CLOSE_CONNECTION	客户端关闭连接
 * */
enum HTTP_CODE{
	NO_REQUEST = 0,
	GET_REQUEST,
	BAD_REQUEST,
	FORBIDDEN_REQUEST,
	INTERNAL_ERROR,
	CLOSE_CONNECTION
};

/*
 *	从一个状态机中, 解析一个行的内容
 * */
LINE_STATUS parse_line(char *buffer, int &check_index, int &read_index){
	char temp;
	/*
	 *	buffer	为读的缓存区
	 *	check_index	当前正在分析的字节
	 *	read_index	客户尾端的下一个字节
	 * */
	for(; check_index < read_index; check_index++){
		temp = buffer[check_index];
		/*
		 *	如果当前为要分析的字节'\r', 则可能说明读取到完整的行
		 * */
		if(temp == '\r'){
			/*
			 *	如果'\r'字符碰巧是目前buffer中的最后一个被读入的客户数据,
			 *那么这次分析没有读取到一个完整的行, 返回LINE_OPEN为读取的行不完整, 还需读取客户数据来分析
			 * */
			if((check_index + 1) == read_index){
				return LINK_OPEN;
			}
			/*
			 *	如果下一个字符'\n', 则说明我们读取到一个完整的行
			 * */
			else if(buffer[check_index + 1] == '\n'){
				buffer[check_index++] = '\0';
				buffer[check_index++] = '\0';
			}
			/*否则的话, 客户端的语法错误*/
			else{
				return LINK_BAD;
			}
		}
		/*如果当前的行'\n', 则说明可能读取到一个完整的行*/
		else if(buffer[check_index] == '\n'){
			if((check_index > 1) && buffer[check_index - 1] == '\r'){
				buffer[check_index - 1] = '\0';
				buffer[check_index++]   = '\0';
				return LINK_OK;
			}
			return LINK_BAD;
		}
	}
	/*
	 *	如果所有的内容分析完毕没有读取到'\r', 则返回LINE_OPEN, 进一步读取客户数据
	 * */
	return LINK_OPEN;
}

/*
 * 分析请求行
 * */
HTTP_CODE parse_requestline(char *temp, CHECK_STATE& checkstate){
	char *url = strpbrk(temp, " \t");
	/*如果行中没有空白字符或'\t', 则HTTP请求错误*/
	if(!url){
		return BAD_REQUEST;
	}
	*url++ = '\0';

	char *methon = temp;
	/*strcasecmp对字符串的大小忽略来比较字符串*/
	if(strcasecmp(methon, "GET") == 0) /* 仅支持GET方法*/{
		printf("The request methon is GET\n");
	}
	else {
		return BAD_REQUEST;
	}

	url += strspn(url, " \t");
	char *version = strpbrk(url, " \t");
	if(!version){
		return BAD_REQUEST;
	}

	*version++ = '\0';
	version += strspn(version, " \t");
	/*仅支持HTTP/1.1*/
	if(strcasecmp(version, "HTTP/1.1") != 0){
		return BAD_REQUEST;
	}
	/*检测URL是否合法*/
	if(strncasecmp(url, "http://", 7) == 0){
		url += 7;
		url = strchr(url, '/');
	}

	if(!url || url[0] != '/'){
		return BAD_REQUEST;
	}

	printf("The request URL is: %s\n", url);
	/*HTTP 请求行处理完毕, 状态转移到头部字段分析*/
	checkstate = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

/*分析头部字段*/
HTTP_CODE parse_headers(char *tmp){
	/*
	 *	遇到一个空行, 说明我们得到了一个正确的HTTP请求
	 * */
	if(tmp[0] == '\0'){
		return GET_REQUEST;
	}
	/*处理HOST头部字段*/
	else if(strncasecmp(tmp, "Host:", 5) == 0){
		tmp += 5;
		tmp += strspn(tmp, " \t");
		printf("the request host is: %s\n", tmp);
	}
	else{
		printf("I can not handle this header\n");
	}
	return NO_REQUEST;
}

/*
 *	分析HTTP请求的入口函数
 * */
HTTP_CODE parse_content(char *buffer, int &check_index, CHECK_STATE &checkstate, int &read_index, int &start_line){
	/*记录当前行的读取状态*/
	LINE_STATUS linestatus = LINK_OK;
	/*记录HTTP请求的处理结果*/
	HTTP_CODE recode = NO_REQUEST;

	/*主状态机, 用于从buffer中取出所有完整的行*/
	while((linestatus = parse_line(buffer, check_index, read_index)) == LINK_OK){
		char *temp  = buffer + start_line; //line在行中的buffer的起始位置
		start_line = check_index;
		switch(checkstate){
			case CHECK_STATE_REQUESTLINE:{	//第一个状态, 分析请求行
											 recode = parse_requestline(temp, checkstate);
											 if(recode = BAD_REQUEST){
												 return BAD_REQUEST;
											 }
											 break;
										 }
			case CHECK_STATE_HEADER:{
										recode = parse_headers(temp);
											if(recode == BAD_REQUEST){
												return BAD_REQUEST;
											}
											else if(recode == GET_REQUEST){
												return GET_REQUEST;
											}
											else{
												break;	
											}				
									}
			default:{
						return INTERNAL_ERROR;		
					}
		}
		/*若没有读取到一个完整的行, 则表示还需要继续读取客户数据才能进一步分析*/
		if(linestatus == LINK_OPEN){
			return NO_REQUEST;
		}
		else {
			return BAD_REQUEST;
		}
	}
}

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
	int main(){
		int sockfd = create_socket();

		struct sockaddr_in client;
		socklen_t cli_len = sizeof(client);
		memset(&cli_len, '\0', cli_len);
		int accept_fd = accept(sockfd, (struct sockaddr*)&client, &cli_len);
		if(accept_fd == -1){
			printf("accept failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
			exit(0);
		}

		char buffer[BUFF_SIZE];
		memset(buffer, '\0', BUFF_SIZE);
		int data_read = 0;
		int read_index = 0; /*当前读取了多少客户的数据字节*/
		int check_index = 0; /*当前已经分析了多少字节的客户数据*/
		int start_buffer = 0; /*行在buffer中的起始位置*/

		/*设置主状态机的初始状态*/
		CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
		while(true){
			data_read = recv(accept_fd, buffer + read_index, BUFF_SIZE - read_index, 0);
			if(data_read == -1){
				printf("reading failure\n");
				break;
			}
			else if(data_read == 0){
				printf("remote client has closed the connection\n");
				break;
			}
			read_index += data_read;
			/*
			 *	分析目前已经获得的所有客户的数据
			 * */
			HTTP_CODE result = parse_content(buffer, check_index, checkstate, read_index, start_buffer);
			if(result == NO_REQUEST){
				continue;
			}
			else if(result == GET_REQUEST){
				send(accept_fd, "ok", 2, 0);
			}
			else {
				send(accept_fd, "error", 5, 0);
				break;
			}
		}
		close(accept_fd);
		close(sockfd);
		return 0;
	}

