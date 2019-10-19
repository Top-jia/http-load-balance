#include <event.h>

#include <iostream>

void myTime(int sock, short event, void *arg){
	
	std::cout << "test " << std::endl;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	struct event ** pt = (struct event**)arg;
	event_add(*pt, &tv);	
}

int main(){
	

	struct event_base *base = event_base_new();

	struct event *pEvent;
    pEvent = event_new(base, -1, EV_TIMEOUT, myTime, &pEvent);

	struct timeval tv;
	tv.tv_sec = 4;
	tv.tv_usec = 0;

	event_add(pEvent, &tv);

	event_base_dispatch(base);

	event_free(pEvent);
	event_base_free(base);

	return 0;
}
