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
/* 设置监听个数*/
#define LISTEN_NUM	10
/* 对epoll创建时的数据进行分装*/
#define EPOLL_NUM	5
/* 对epoll的所监听的文件描述符的个数*/
#define EPOLL_EVENT_NUM	1024
/* 对读取的字符串的大小的设置*/
#define BUFF_SIZE	127
/*全局共享内存的标识*/
#define SHARE_MEMRAY_KEY	"1234"


/*信号处理函数*/
void sig_handler(int sig){
	int save_errno = errno;
	int msg = sig;
	/*给管道中写入信号值*/
	send(sockpair_fd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}
/*创建socket地址协议族*/
int create_socket(){
	const char *ip = "127.0.0.1";
	const int port = 7900;
	
	struct sockaddr_in server;
	memset(&server, '\0', sizeof(server));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, server.sin_addr);
	server.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd != -1){
		fprintf(stderr, "create_socket()__socket() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	/*避免timewait状态*/
	int reuse = -1;
	int err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	if(err == -1){
		fprintf(stderr, "create_socket()__setsockopt() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int binder = bind(sockfd, (struct sockaddr*)&server, sizeof(struct sockaddr));
	if(binfer == -1){
		fprintf(stderr, "create_socket()__bind() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	binder = listen(sockfd, LISTEN_NUM);
	if(binder == -1){
		fprintf(stderr, "create_socket()__listen() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

/*对epoll管理的描述符, 进行分装*/
class Epoll{
		int epoll_fd;
		//std::stack<int > fd_set; 应该增加, 当程序结束的时候来释放其存在的文件描述符所占的资源.
	public:
		/*  对描述符设置非阻塞模式*/
		static void setnonblocking(int fd){
			if(fd <= 0){
				fprintf(stderr, "Epoll::setnonblocking() args failure");
				exit(EXIT_FAILURE);
			}
			int old_option = fcntl(fd, F_GETFL);
			if(old_option == -1){
				fprintf(stderr, "Epoll:setnonblocking()__fcntl() failure errno = %d strerror = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			int new_option = old_option | O_NONBLOCK;
			int err = fcntl(fd, F_SETFL, new_option);
			if(errr == -1){
				fprintf(stderr, "Epoll:setnonblocking()__fcntl() falure errno = %d strerror = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*初始化相关Epoll构造函数*/
		Epoll(){
			epoll_fd = epoll_create(EPOLL_NUM);
			if(epoll_fd == -1){
				fprintf(stderr, "Epoll::Epoll()__epoll_create() failure errno = %d strerror = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*增加事件到Epoll中*/
		void addfd(int fd, struct epoll_event *event){
			if(fd <= 0){
				fprintf(stderr, "Epoll()::addfd() args failure\n");
				exit(EXIT_FAILURE);
			}
			if(event != NULL){
				int err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
				if(err == -1){
					fprintf(stderr, "Epoll::addfd()__epoll_ctl() failure errno = %d strerror = %s\n", errno, strerror(errno));
					exit(EXIT_FAILURE);
				}
				return;
			}
			/*默认事件类型为 EPOLLIN EPOLLET EPOLLRDHUP*/
			struct epoll_event self_event;
			memset(&self_event, '\0', sizeof(struct epoll_event));
			self_event.data.fd = fd;
			self_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
			int  err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &self_event);
			if(err == -1){
				fprintf(stderr, "Epoll::addfd()__epoll_ctl() failure errno = %d, strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*移除文件描述符*/
		void removefd(int fd){
			if(fd <= 0){
				fprintf(stderr, "Epoll::removefd() args failure\n");
				exit(EXIT_FAILURE);
			}
			int err = epoll_ctl(epoll_fd. EPOLL_CTL_DEL, fd, NULL);
			if(err == -1){
				fprintf(stderr, "Epoll::removefd()__epolll_ctl() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*修改文件描述符*/
		void modfd(int fd, struct epoll_event *event){
			if(fd <= 0 || event == NULL){
				fprintf(stderr, "Epoll::modfd() args failure\n");
				exit(EXIT_FAILURE);
			}
			int err = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event);
			if(err == -1){
				fprintf(stderr, "Epoll::modfd()__epoll_ctl() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*析构相关的资源*/
		~Epoll(){
			close(epoll_fd);
			/*其中里面的文件描述符, 仍未释放*/
		}
		/*获得Epoll文件描述符*/
		int getepoll_fd(){
			return epoll_fd;
		}
};

/*对于Epoll事件的处理数据类*/
Ddata class{
		struct epoll_event events[EPOLL_EVENT_NUM];
		/*与子进程通信的双端管道*/
		int *sockpair_fd;
		Epoll epoll_data;
		/*主程序的网络的fd*/
		int sockfd;
		int client_fd;
		/*判断主程序是否运行*/
		bool master_run;
		/*进程池*/
		Sprocess *sub_process;
	public:
		/*构造Ddata类: 初始化成员变量*/
		Ddata(int _sockfd):sockfd(_sockfd), master_run(true){
			sockpair_fd = new int[2];
			int err = socketpair(PF_UNIX, SOCK_STREAM, 0, sockpair);
			if(err == -1){
				fprintf(stderr, "Ddata::Ddata()__socketpair() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			memset(events, '\0', sizeof(struct epoll_event) * EPOLL_EVENT_NUM);
			/*先进行一次连接客户端*/
			struct sockaddr_in client;
			socklen_t cli_len = sizeof(struct sockaddr_in);
			memset(&client, '\0', cli_len);
			client_fd = accept(sockfd, (struct sockaddr*)&client, &cli_len);
			if(client_fd == -1){
				fprintf(stderr, "Ddata::Ddata()__accept() failure errno = %d, strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			/*对主网络连接的描述符进行设计*/
			epoll_data.setnonblocking(sockfd);
			struct epoll_event event;
			memset(&event, '\0', sizeof(struct epoll_event));
			event_data.data.fd = sockfd;
			event_data.event = EPOLLIN | EPOLLRDHUP;
			epoll_data.addfd(sockfd, &event);

			/*对主网络连接的管道进行设计*/
			memset(&event, '\0', sizeof(struct epoll_event));
			event_data.data.fd = sockpair_fd[0];
			event_data.event = EPOLLIN | EPOLLRDHUP;
			epoll_data.addfd(sockpair_fd[0], &event);
		}
		/*关闭相关的资源, 析构函数*/
		~Data(){
			close(sockpair_fd[0]);
			close(sockpair_fd[1]);
			close(sockfd);
			delete []sockpair;
		}
		/*增加信号*/
		void addsig(int sig, void(*handler)(int)){
			if(sig < 0 || handler == NULL){
				fprintf(stderr, "Ddata::addsig() failure args\n");
				exit(EXIT_FAILURE);
				struct sigaction sa;
				memset(&sa, '\0', sizeof(struct sigaction));
				sa.sa_handler = handler;
				sigfillset(&sa.sa_mask);
				int err = sigaction(sig, &sa, NULL);
				if(err == -1){
					fprintf(stderr, "Ddata::addsig()__sigaction() failure errno = %d strerror(errno) = %s, sig = %d, strsignal(sig) = %s", errno, strerror(errno), sig, strsignal(sig));
					exit(EXIT_FAILURE);
				}
			}
		}
		/*设置信号处理*/
		void hand_signal(){
			/*将一些信号增加到master进程中, 并在子进程中将其屏蔽
			 *	SIGCHLD		子进程如果死亡, 应该避免僵尸进程的产生
			 *	SIGTERM		请求终止的时候, 由kill进行发出
			 *	SIGINT		Ctrl+c产生的进程中断处理操作
			 *	SIGPIPE		管道关闭, 然后进行写操作/或读操作
			 * */
			addsig(SIGCHLD, sig_handler);
			addsig(SIGTERM, sig_handler);
			addsig(SIGINT,  sig_handler);
			addsig(SIGPIPE, SIG_ING);
		}
		/*运行程序*/
		void run(){
			handle_signal();
			while(master_run){
				int epoll_num = epoll_wait(epoll_data.getepoll_fd(), events, EPOLL_EVENT_NUM, -1);{
					/*epoll系统调用, 错误情况的处理*/
					if(epoll_num == -1){
						/*这个错误时, 是在陷入内核的epoll, 此时接受到了信号所发生的错误*/
						if(errno == EINTR){
							continue;
						}
						fprint(stderr, "Ddata::run()__epoll_wait() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
						exit(EXIT_FAILURE);
					}
					else if(epoll_num == 0){
						fprintf(stderr, "this is not happend\n");
						continue;
					}
					else{
						/*有事件到来*/
						other_handle_epoll(epoll_num);
					}
				}
			}
		}

		/*epoll中有多个文件描述符返回, 并对其中的描述符进行处理*/
		void other_handle_epoll(int epoll_num){
			if(epoll_num <= 0){
				continue;	
			}
			
			for(int i = 0; i < epoll_num; i++){
				int fd = events[i].data.fd;
				/*有客户端数据到来*/
				if(fd == sockfd){
					
				}
			
			
			}
		}
		/*声明信号处理函数为友元*/
		friend void sig_handler(int sig);
};

int mainn(){
	int sockfd = create_socket();
	Data data(sockfd);
	data.run();
	return 0;
}
