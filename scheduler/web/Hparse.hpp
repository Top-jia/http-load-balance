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

#define BUFF_SIZE	127
#define HTTP_BUFF	1024

/*
 *	这是一个简单的解析类(http), 解析http中所要的文件,
 *并将其中的文件进行打包, 并返回, 其中根据所得到的文件
 *的状态. 返回其中的报文.在断开, 用主从状态机进行分析.
 * 	GTE支持
 * */
/*	
 *	主状态机只有两种状态, 正在请求头部字段, 或者请求行.
 * */
enum CHECK_STATE{
	CHECK_STATE_REQUESTLINE = 0,
	CHECK_STATE_HEADER
};

/*
 *	从服务器是请求一个行的状态, 一个完整的行, 不完整的行, 行错误
 * */
enum LINE_STATUS{
	LINE_GOOD = 0,
	LINE_UCOMPLETION,
	LINE_ERROR
};

/*
 *	请求的HTTP的结果:
 *			NO_REQUEST		请求不完整
 *			GET_REQUEST		获得一个完整的请求
 *			BAD_REQUEST		请求的客户端语法错误
 *			INTERNAL_ERROR	请求的内部错误(服务器)
 *			CLOSE_CONNECTION	客户端已经关闭连接
 * */
enum HTTP_CODE{
	NO_REQUEST = 0,
	GET_REQUEST,
	BAD_REQUEST,
	INTERNAL_REEOR,
	CLOSE_CONNECTION
};
/*
 *	这个类是内部解析类, 其中根据内部的状态码, 
 *来判断, 并且来返回数据
 * */
class Hparse{
	public:
		char buffer[HTTP_BUFF];
		/* recv中每次读取的数据个数的返回值*/
		int data_read;
		/* 当前已经读取了多少的客户数据*/
		int read_index;
		/* 当前已经分析完的客户数据*/
		int check_index;
		/* 行在buffer中的起始位置*/
		int start_line;
		/* 主读取状态的表示*/
		CHECK_STATE checkstate;
		/* 请求文件的位置*/
		std::string location;
		int accept_fd;
	Hparse(int fd);
	/* 主循环读取数据, 并根据读取的数据分析, */
	HTTP_CODE main_loop();
	HTTP_CODE parse_content();
	HTTP_CODE parse_headers();
	HTTP_CODE parse_requestline();
	LINE_STATUS parse_line();
	/* 返回数据的处理, 简单发送一个报文*/
	void reply_http()
};
