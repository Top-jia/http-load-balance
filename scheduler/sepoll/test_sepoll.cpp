#include"sepoll.hpp"

int main(){
	Logger log;
	Sepoll epoll(log);
	
	epoll.addfd(7);
	epoll.addfd(8);

	epoll.delfd(7);
	epoll.setnonblocking(7);
	
	return 0;
}
