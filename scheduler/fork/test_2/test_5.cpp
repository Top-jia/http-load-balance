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
/* 全局共享内存的标识*/
#define SHARE_MEMRAY_KEY	"1234"
/* 所创建的子进程数量*/
#define SUB_PROCESS_NUM		5
/*对于每一个子进程在共享内存端, 都进行了偏移*/
#define SHARE_MEMORY_CHUNK	256


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

/*对于共享内存的控制类的设计*/
Smemory class{
		void *share_memory;
		int  share_length;
		int  shm_id;
	public:
		/*获得共享内存的长度, 并创建共享内存的标识*/
		Smemory(int len = SHARE_MEMORY_CHUNK):share_length(len), share_memory(NULL), shm_id(-1){
			shm_id = shmget((key_t)SHARE_MEMORY_KEY, share_length, 0666 | IPC_CREAT);
			if(shm_id == -1){
				fprintf(stderr, "Smemory::Smemory()__shmget() failure errno = %d, strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*获取共享内存的地址*/
		void my_shmat(){
			share_memory = shmat(shm_id, NULL, 0);
			if((void *)share_memory == -1){
				fprintf(stderr, "Smemory::my_shmat()__shmat() failure errno = %d, strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*从进程中将共享内存的这块地址与子进程分离开*/
		void my_shmdt(){
			int err = shmdt(share_memory);
			if(err == -1){
				fprintf(stderr, "Smemory::my_shmdt()__shmdt() failure errno = %d, strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*从进程中删除共享内存的这块地址*/
		void my_shm_remove(){
			int err = shmctl(shm_id, IPC_RMID, 0);
			if(err == -1){
				fprintf(stderr, "Smemory::my_shm_remove()__shmctl() errno = %d, strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		/*获取共享内存的的地址*/
		void* get_share_memory(){
			return share_memory;
		}
		/*对共享内存的这块地址进行修改: 做哪方面的修改*/
		void my_shmctl(){
		}
		/*对共享内存的操作---不做修改*/
		~Smemory(){		
		}
};

/*对于子进程的信号处理函数*/
void sub_sig_handler(int sig){
	int save_errno = errno;
	int msg = sig;
	/*给管道中写入信号值*/
	send(sockpair_fd, (char*)&msg, 1, 0);
	errno = save_errno;
}

/*对于子进程的处理设计*/
Sprocess class{
		/*和主进程通信用的双端管道的fd*/
		int sockpair_fd;
		/*用epoll来对文件描述符, 进行监听, 可能还会有定时事件的加入.*/
		void *share_memory;
		/*用epoll来监听管道的文件描述符*/
		Epoll edata;
		/*epoll_event的事件集, 只包含一个管道的事件*/
		struct epoll_event events[MAX_EVENT_NUM];
		/*判断子进程是否运行*/
		bool sub_flag;
		/*标识子进程是否在处理数据*/
		bool being_process;
	public:
		/*对于子进程来说, 其中共享内存有自己的默认值, 其中应该对信号进行一定的设计*/
		Sprocess(int sockpairfd, void *_share_memory):sockpair_fd(sockpairfd), share_memory(_share_memory), sub_flag(true), begin_process{
			/*注册一个管道描述符到epoll中*/
			edata.setnonblocking(sockpairfd);
			/*sockpair_fd的事件必须自己设计, 不能含有EPOLLET模式*/
			struct epoll_event event;
			memset(&event, '\0', sizeof(struct epoll_event));
			event.data.fd = sockpair_fd;
			event.events  = EPOOLLIN;
			edata.addfd(sockpair_fd);
			/*对于信号的设计, 只设计一个信号
			 *	SIGTERM		当从kill发出时候, 就对其进行子进程移除操作.
			 * */
			sub_addsig(SIGTERM, sub_sig_handler);
		}
		/*运行子进程*/
		void run_child(){
			while(sub_flag){
				int num = epoll_wait(edata.get_epollfd(), events, MAX_EVENT_NUM, -1);
				if(num <= 0){
					if(errno == EINTR){
						continue;
					}
					fprintf(stderr, "Sprocess::run_child() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
					exit(EXIT_FAILURE);
				}
				else{
					/*定义读取管道(非阻塞)的规则:
					 *	1.主进程可能通知有数据在共享内存中
					 *	2.有信号的触发, 会发送信号,
					 *	所以:
					 *		struct event_type{
					 *			change == 0	为子进程信号事件
					 *			change == 1 为父进程信号事件
					 *			change == 2	为主进程通知(共享内存)的数据, 要子进程处理数据，或相反.
					 *			int change;
					 *			int sub_signum;
					 *			int father_signum;
					 *			int message;
					 *		};
					 * */
					for(int i = 0; i < num; i++){
						int fd = events.data.fd;
						if(fd == sockpair_fd && events.events & EPOLLIN){
							while(true){
								struct event_type etype;
								memset(&etype, '\0', sizeof(struct event_type));
								int read_data = read(fd, (void*)&etype, sizeof(struct event_type), 0);
								if(read_data < 0){
									/*将数据读完*/
									if(errno == EAGAIN || errno == EWOULDBLOCK){
										break;
									}
								}
								/*处理信号事件*/
								if(etype.change == 0){
									sub_flag = false;							
								}
								/*处理主进程给子进程发送的数据*/
								else if(etype.change == 2)
								{
									memory((char*)share_memory, '\0', SHARE_MEMORY_CHUNK);
									strncpy((char*)share_memory, "hello world", 11);
									etype.change = 2;
									write(sockpair_fd, (char*)&etype, sizeof(struct event_type));
								}
								/*可能还有时间轮机制, 来维持长连接, 暂时先放下*/
								else{
								}
							}
						}
					}
				}
			}		
		}
		/*释放子进程的所有的资源*/
		~Sprocess(){
			/*子进程中与共享内存分离操作, 不进行移除操作(主进程负责移除操作)*/
			int err = shmdt(share_memory);
			if(err == -1){
				fprintf(stderr, "Sprocess::~Sprocess()_shmdt() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
				exit(EXIT_FAILURE);
			}
			/*关闭双端管道, 还有一端(任然未释放)*/
			close(sockpair);	
		}
}
		/*对子进程的信号处理*/
		void sub_addsig(int sig, void(*handler)(int)){
			if(sig < 0 || handler == NULL){
				fprintf(stderr, "Ddata::addsig() failure args\n");
				exit(EXIT_FAILURE);
			}
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
		/*信号处理函数*/
		friend void sub_sig_handler(int sig);
};
/*创建子进程的类, 并对子进程, 如过不对其修改则地址不改变*/
void create_sub_process(Sprocess &*ptr, int sockpair, void *sub_share_memory){
		pid_t pid = fork();
		if(pid == -1){
			fprintf(stderr, "create_sub_process()__fork() failure errno = %d strerror(errno) = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		else if(pid > 0){
			return ;
		}
		else{
			ptr = new Sprocess(sockpair, sub_share_memory);
			ptr->sub_run();
		}
		return;
}
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
		Sprocess **sub_process;
		/*对于共享内存的设计*/
		Smemory share_memory;
	public:
		/*构造Ddata类: 初始化成员变量*/
		Ddata(int _sockfd):sockfd(_sockfd), master_run(true), sub_process(NULL){
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
			/*对主网络连接的描述符进行设计, 连接调度器的主网络*/
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

			/*对子进程的处理设计*/
			sub_process = new Sprocess *[sub_process];
			for(int i = 0; i < sub_process; i++){
				/*暂时不考虑资源的释放问题, 先解决流程*/
				/*用一个函数来创建子进程, 防止在父进程中仍有多余的sub_process, sub_process只是子进程调用的开头*/
				create_sub_process(sub_process[i], sockpair_fd[1], i * SHARE_MEMORY_CHUNK + share_memory.get_share_memory());
			}
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
			}
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
			/*处理相关的信号*/
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
				/*有客户端数据到来:
				 * 	1.如何表示那个地址, 来交给子进程.
				 * 		给地址分段方法, 来处理
				 * 		share_memory + SHARE_MEMEOY_CHUNK*i;分给某个地址
				 *  2.如何确定子进程是否正在处理.
				 *  	设计信号量组来同步一个, 来表示进程之间对同一变量是否在用.全局的
				 *  	bool sub_process_is_use = false;
				 * */
				if(fd == sockfd){
					struct event_type etype;
					memset(&etype, '\0', sizeof(struct event_type));
					etype.change = 2;

				}
				/*关于父进程处理的管道:
				 *		1.父进程中信号的写入.
				 *		2.子进程给父进程发送的数据.
				 * */
				else if(fd == sockpair_fd[0])
			
			
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
