#include<iostream>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdbool.h>
#include<libgen.h>
#include<signal.h>
#include<bits/signum.h>
#include<bits/sigset.h>
#include<sys/epoll.h>

#define BUFF_SIZE	127
#define MAX_EVENT_NUM	1024

/*父进程用于和父进程信号通信的文件描述符*/
int parent_sig_read_fd = 0;
int parent_sig_write_fd = 0;
int user_count = 0;

#define MAX_PROCCESS	5

/*创建socket协议族 */
int create_socket(){
	const char *ip = "127.0.0.1";
	const int port = 7900;

	struct sockaddr_in server;
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &server.sin_addr);
	server.sin_port = htons(port);

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd  == -1){
		fprintf(stderr, "create_socket() failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	int reuse = -1;
	int err = setsockopt(sockfd, SOCK_STREAM, SO_REUSEADDR, &reuse, sizeof(reuse));
	if(err == -1){
		fprintf(stderr, "setsockopt() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int binder = bind(sockfd, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	if(binder == -1){
		fprintf(stderr, "bind() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	binder = listen(sockfd, 5);
	if(binder == -1){
		fprintf(stderr, "listen() failure errno = %d std_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

/*设置信号处理函数, 给管道中写入每一个信号值*/
void sig_handler(int signum){
	int err = errno;
	int msg = signum;
	printf("pid = %d catch signum = %d, str_sig = %s\n", getpid(), signum, strsignal(signum));
	send(parent_sig_write_fd, (char*)&msg, 1, 0);
	if(errno == EBADF){
		printf("子进程pid = %d, send--fd failure\n", getpid());
	}
	errno = err;
}


/*设置对信号处理的类*/
class Signal{
	int *sockpair;
	public:
	/*构造信号类, 并创建双端管道用于, 将信号事件作为统一事件源的一种来处理*/
	Signal():sockpair(NULL){
		sockpair = new int[2];
		int err = socketpair(PF_UNIX, SOCK_STREAM, 0, sockpair);
		if(err == -1){
			fprintf(stderr, "socketpair() failure errno = %d std_err = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	/*释放创建管道文件的描述符*/
	~Signal(){
		delete[] sockpair;
	}
	/*对于描述符分为读端和写端, sockpair[0]作为写端, sockpair[1]作为读端*/
	int getsockpair_read(){
		return sockpair[1];
	}
	/*对于描述符分为读端和写端, sockpair[0]作为写端, sockpair[1]作为读端*/
	int getsockpair_write(){
		return sockpair[0];
	}
	/*增加信号,(就是对一些信号处理函数进行按照自己的方式重写)
	 *	例如 SIGINT ctrl+C来进行控制的.
	 * */
	void addsig(int signum){
		struct sigaction sa;
		memset(&sa, '\0', sizeof(sa));
		sa.sa_handler = sig_handler;
		sa.sa_flags |= SA_RESTART;	//支持阻塞的系统调用, 被信号中断后, 可以重新恢复
		int err = sigfillset(&sa.sa_mask);
		if(err == -1){
			fprintf(stderr, "Signal::addsig()--sigfillset() failure errno = %d, str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		err = sigaction(signum, &sa, NULL);
		if(err == -1){
			fprintf(stderr, "Signal::addsig()--sigaction() failure errno = %d, str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	/*
	 * 对于某个信号恢复其默认值(默认行为)
	 * */
	void delsig(int signum){
		struct sigaction sa;
		memset(&sa, '\0', sizeof(sa));
		//sa.sa_flags |= SA_RESTART;
		sa.sa_handler = SIG_DFL;
		int err = sigfillset(&sa.sa_mask);
		if(err == -1){
			fprintf(stderr, "Signal::delsig()--sigfillset() failure errno = %d, str_err = %s", errno, strerror(errno));
		}
		err = sigaction(signum, &sa, NULL);
		if(err == -1){
			fprintf(stderr, "Signal::addsig()--sigaction() failure errno = %d, str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
};

/*封装Epoll来处理事件*/
class Epoll{
	int epoll_fd;
	public:
	/*初始化Epoll构造函数*/
	Epoll(){
		epoll_fd = epoll_create(5);
		if(epoll_fd == -1){
			fprintf(stderr, "Epoll::Epoll()--epoll_create() failure errno = %d, str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	/*析构函数*/
	~Epoll(){
	}

	/*设置非阻塞模式*/
	static void setnonblocking(int fd){
		if(fd <= 0){
			fprintf(stderr, "Epoll:setnonblocking() failure in args");
			exit(EXIT_FAILURE);
		}
		int old_option = fcntl(fd, F_GETFL);
		if(old_option == -1){
			fprintf(stderr, "Epoll::setnonblocking() failure errno = %d, str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		int new_option = old_option | O_NONBLOCK;
		int err = fcntl(fd, F_SETFL, new_option);
		if(err == -1){
			fprintf(stderr, "Epoll::setnonblocking() failure errno = %d, str_er = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	/*增加文件描述符*/
	void addfd(int fd, struct epoll_event *event = NULL){
		if(fd <= 0){
			fprintf(stderr, "Epoll::addfd() failure in args");
			exit(EXIT_FAILURE);
		}
		if(event == NULL){
			struct epoll_event event;
			bzero(&event, sizeof(struct epoll_event));
			event.data.fd = fd;
			event.events = EPOLLIN | EPOLLRDHUP;
			int err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
			if(err == -1){
				fprintf(stderr, "Epoll::addfd()--epoll_ctl() failure errno = %d, str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		else{
			int err = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
			if(err == -1){
				fprintf(stderr, "Epoll::addfd()--epoll_ctl() failure errno = %d, str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}

	/*修改描述符fd*/
	void modfd(int fd, struct epoll_event *event){
		if(fd <= 0 || event == NULL){
			fprintf(stderr, "Epoll::modfd() failure in args");
			exit(EXIT_FAILURE);
		}
		int err = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event);
		if(err == -1){
			fprintf(stderr, "Epoll::modfd() failure errno = %d str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/*删除描述符fd*/
	void delfd(int fd){
		if(fd <= 0){
			fprintf(stderr, "Epoll::delfd() failure in args");
			exit(EXIT_FAILURE);
		}
		int err = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		if(err == -1){
			fprintf(stderr, "Epoll::delfd()--epoll_ctl() failure in args");
			exit(EXIT_FAILURE);
		}
	}

	/*获得epoll_fd内核事件文件描述符的fd*/
	int getfd(){
		return epoll_fd;
	}
};


/*设置处理信号的类, 并且为处理数据
 *	接口留下, 先完成信号的测试
 * */
class Pdata{
	Epoll &epoll_info;
	int sockfd;
	int parent_sig_read_fd;

	struct epoll_event events[MAX_EVENT_NUM];
	public:
	/*初始化相关的数据和结构*/
	Pdata(Epoll &epollinfo, int _sockfd, int sig_pipe):\
		epoll_info(epollinfo), sockfd(_sockfd), parent_sig_read_fd(sig_pipe){
			bzero(&events, sizeof(struct epoll_event) * MAX_EVENT_NUM);			
		}
	/*主要运行的函数*/
	void run(){
		while(true){
			int epoll_return = epoll_wait(epoll_info.getfd(), events, MAX_EVENT_NUM, -1);
			if(epoll_return <= 0 && errno != EINTR){
				fprintf(stderr, "Pdata::run()--epoll_wait() failure errno = %d str_err = %s", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else{
				for(int i = 0; i < epoll_return; i++){
					int fd = events[i].data.fd;
					/*有请求连接的事件到来*/
					if(fd == sockfd){
						accept_link(fd);
					}
					/*监控信号的事件文件描述符, 有事件触发*/
					else if(fd == parent_sig_read_fd){
						deal_sig_pipe(fd);
					}
					else{
						printf("otherthing happened\n");
						continue;
					}
				}
			}
		}
	}
	/*有连接事件到来*/
	void accept_link(int fd){
		if(fd <= 0){
			fprintf(stderr, "Pdata::accept_link() failure in args");
			exit(EXIT_FAILURE);
		}
		struct sockaddr_in client;
		socklen_t socklen = sizeof(client);
		bzero(&client, socklen);
		int accept_fd = accept(fd, (struct sockaddr*)&client, &socklen);
		if(accept_fd == -1){
			fprintf(stderr, "Pdata::accept_link()--accept() failure errno = %d str_err = %s", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		/*如果连接用户过多的情况下*/
		if(user_count >= MAX_PROCCESS){
			const char *info = "too many users\n";
			printf("%s", info);
			send(accept_fd, info, strlen(info), 0);
			close(accept_fd);
			return;
		}

		pid_t pid = fork();
		if(pid < 0){
			close(accept_fd);
			return;
		}
		/*子进程*/
		else if(pid == 0){
			epoll_info.~Epoll();
			close(sockfd);
			close(parent_sig_read_fd);
			close(parent_sig_write_fd);
			run_child(accept_fd);
			exit(EXIT_SUCCESS);
		}
		/*父进程*/
		else{
			return;	
		}
	}

	/*运行子进程的函数*/
	void run_child(int fd){
		if(fd <= 0){
			fprintf(stderr, "Pdata::run_child() failure in args");
			exit(EXIT_FAILURE);
		}
		char buffer[BUFF_SIZE];
		Epoll::setnonblocking(fd);
		while(true){
			memset(buffer, '\0', sizeof(char) * BUFF_SIZE);
			int recv_len = recv(fd, buffer, BUFF_SIZE, 0);
			if(recv_len == -1 && (errno != EAGAIN || errno != EWOULDBLOCK)){
				printf("pid = %d Pdata::run_child()--recv() failure errno = %d str_err = %s\n", getpid(), errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			else{
				if(strlen(buffer) == 0){
					continue;
				}
				printf("pid = %d recv_data = %s\n", getpid(), buffer);
				if(strncmp("end", buffer, 3) == 0){
					close(fd);
					exit(EXIT_FAILURE);			
				}
				send(fd, buffer, strlen(buffer), 0);
				memset(buffer, '\0', strlen(buffer));
			}
		}
	}

	/*处理管道中的信号事件*/
	void deal_sig_pipe(int sig_pipe_fd){
		if(sig_pipe_fd <= 0){
			fprintf(stderr, "Pdata::deal_sig_pipe() failure in args");
			exit(EXIT_FAILURE);
		}
		char signal[BUFF_SIZE];
		bzero(&signal, sizeof(char) * BUFF_SIZE);
		/*读取管道中触发了那些信号值*/
		int ret = recv(sig_pipe_fd, signal, BUFF_SIZE - 1, 0);
		if(ret <= 0){
			fprintf(stderr, "Pdata::deal_sig_pipe()--recv() failure errno = %d str_err = %s", errno, strerror(errno));
		}
		else{
			for(int i = 0; i < ret; i++){
				switch(signal[i]){
					case SIGCHLD:
						printf("SIGCHLD is trigger\n");
						break;
					case SIGHUP:
						printf("SIGHUP is trigger\n");
						break;
					case SIGQUIT:
						printf("SIGQUIT is trigger\n");
						break;
					case SIGINT:
						printf("SIGINT is trigger\n");
						break;
					case SIGTERM:
						printf("SIGTERM is trigger\n");
						break;
					default :
						printf("default sig handler\n");
						break;
				}
			}

		}
	}
};
/*主函数*/
int main(){
	int sockfd = create_socket();
	Signal signal_set;
	parent_sig_read_fd = signal_set.getsockpair_read();
	parent_sig_write_fd = signal_set.getsockpair_write();

	Epoll epoll_info;
	epoll_info.setnonblocking(parent_sig_read_fd);
	epoll_info.addfd(parent_sig_read_fd);
	epoll_info.setnonblocking(sockfd);
	epoll_info.addfd(sockfd);
	/*
	 *	对那些信号进行重新设计其回调函数
	 *	strsignal(signum); //回复信号的字符串
	 *	
	 *	SIGINT		ctrl+c(中断进程)
	 *	SIGQUIT		ctrl+\(键盘输入使进程退出)
	 *  SIGHUP		ctrl+z(控制终端挂起)
	 *	SIGTERM		kill杀死进程
	 *	SIGCHLD		子进程状态发生变化
	 * */
	signal_set.addsig(SIGINT);
	signal_set.addsig(SIGQUIT);
	signal_set.addsig(SIGHUP);
	signal_set.addsig(SIGTERM);
	signal_set.addsig(SIGCHLD);

	Pdata ddata(epoll_info, sockfd, parent_sig_read_fd);
	ddata.run();
	return 0;
}
