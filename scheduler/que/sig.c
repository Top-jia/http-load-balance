
#define _GNU_SOURCE        /* or _BSD_SOURCE or _SVID_SOURCE */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include<time.h>

#define MSG "OMG , I catch the signal SIGINT\n"
#define MSG_END "OK,finished process signal SIGINT\n"
int do_heavy_work(){
	int i ;
	int k;
	srand(time(NULL));
	for(i = 0 ; i < 100000000;i++)
	{
		k = rand()%1234589;
	}
	return 0;
}
void signal_handler(int signo){
	write(2,MSG,strlen(MSG));
	do_heavy_work();
	write(2,MSG_END,strlen(MSG_END));
}
int main(){
	char input[1024] = {0};
#if defined SYSCALL_SIGNAL_API
	if(syscall(SYS_signal ,SIGINT,signal_handler) == -1)
#elif defined SYSV_SIGNAL_API
		if(sysv_signal(SIGINT,signal_handler) == SIGERR)
#elif defined BSD_SIGNAL_API
			if(bsd_signal(SIGINT,signal_handler) == SIGERR)
#else
				if(signal(SIGINT,signal_handler) == SIG_ERR)
#endif
				{
					fprintf(stderr,"signal failed\n");
					return -1;
				}
	printf("input a string:\n");
	if(fgets(input,sizeof(input),stdin)== NULL)
	{
		fprintf(stderr,"fgets failed(%s)\n",strerror(errno));
		return -2;
	}
	else
	{
		printf("you entered:%s",input);
	}
	return 0;
}
