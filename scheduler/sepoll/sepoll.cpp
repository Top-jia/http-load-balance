#include"sepoll.hpp"

extern Logger log;

/*
 *	Sepoll() 构造函数初始化
 * */
Sepoll::Sepoll(){
	epoll_fd = epoll_create(5);
	if(epoll_fd == -1){
		log.WriteFile(true, errno, "Sepoll::init_epoll_create failed");
	}
}

/*
 *	对加入epoll事件集的fd, 将其设置为ET模式, 写入可读事件
 * */
void Sepoll::addfd(FD fd, struct epoll_event *event){
	if(fd <= 0){
		log.WriteFile(true, 0, "Sepoll::addfd args fd failed");
	}
	if(event == NULL){
		struct epoll_event event;
		event.data.fd = fd;
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
		if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event)){
			log.WriteFile(true, errno, "Sepoll::addfd_epoll_ctl failed");
		}
	}
	else{
		if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event)){
			log.WriteFile(true, errno, "Sepoll::addfd_epoll_ctl failed");
		}
	}
}

/*
 *	将事件集中的fd, 进行移除操作
 * */
void Sepoll::delfd(FD fd){
	if(fd <= 0){
		log.WriteFile(true, 0, "Sepoll::delfd args fd failed");
	}
	
	if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL)){
		log.WriteFile(true, errno, "Sepoll::delfd_epoll_ctl failed ");
	}
}

/*
 *	将fd的事件, 进行修改
 * */
void Sepoll::modfd(FD fd, struct epoll_event *event){
	if(fd <= 0 || event == NULL){
		log.WriteFile(true, 0, "Sepoll::modfd args fd failed");
	}

	if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, event)){
		log.WriteFile(true, errno, "Sepoll::modfd_epoll_ctl failed");
	}
}

/*
 *	对文件描述符进行设置, 非阻塞模式
 * */
void Sepoll::setnonblocking(FD fd){
	if(fd <= 0){
		log.WriteFile(true, 0, "Spell::setnonblocking failed");
	}

	int old_option = fcntl(fd, F_GETFL);
	if(-1 == old_option){
		log.WriteFile(true, errno, "Sepoll::setnonblocking_fcntl failed ");
	}

	int new_option = old_option | O_NONBLOCK;
	if(-1 == fcntl(fd, F_SETFL, new_option)){
		log.WriteFile(true, errno, "Sepoll::setnonblocking_fcntl failed ");
	}
}

/*
 *	得到epoll_fd的文件描述符
 * */
FD Sepoll::getFD(){
	return epoll_fd;
}
