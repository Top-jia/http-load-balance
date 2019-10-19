#include <event.h>
#include <iostream>

void onTime(int sock, short event, void *arg){
	std::cout << "New Game Over..." << std::endl;

	struct event **Pevent = (struct event**)arg;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	event_add(*Pevent, &tv);
}



int main(){
	struct event_base *base = event_base_new();

	struct event *pEvent ;
	pEvent = event_new(base, -1, EV_TIMEOUT, onTime, &pEvent);
	
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	event_add(pEvent, &tv);

	event_base_dispatch(base);

	event_free(pEvent);
	event_base_free(base);
	return 0;
}
