#include<iostream>
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdbool.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<signal.h>
#include<sys/mman.h>
#include<sys/wait.h>
#include<sys/stat.h>

/*	此程序的目地:
 *		使用多进程提供服务: 
 *			每一个进程为一个客户端服务.
 *			使用双端管道进行父子进程进行数据的通知.
 *			信号的通知.
 *			共享内存的使用.
 *
 *		分三步走:
 *			第一步: 书上的代码看懂,(并模仿的写, 将自己的结构进行设计出来)
 *			第二部: 实现类似的功能,(大结构都使用上)
 *			第三部: 加上时间轮机制(维持其长连接), 并进一步来封装其结构.
 * */

