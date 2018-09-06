#include<stdio.h>
#include<string.h>
#include<signal.h>
#include<bits/signum.h>
#include<bits/sigset.h>
#include<unistd.h>
#include<stdlib.h>

static void sig_int(int signum){
	printf("catch SIGINT\n");
	if(signal(signum, SIG_DFL) == SIG_ERR){
		printf("signal\n");
	}
}

int main(){
	sigset_t new_set, old_set, pend_mask;
	if(signal(SIGINT, sig_int) == SIG_ERR){
		perror("signal\n");
	}

	if(sigemptyset(&new_set) < 0){
		perror("sigempty\n");
	}

	if(sigaddset(&new_set, SIGINT) < 0){
		perror("sigaddset\n");
	}

	if(sigprocmask(SIG_BLOCK, &new_set, &old_set) < 0){
		perror("sigprocset\n");
	}

	printf("\nSIGINT block\n");
	sleep(5);

	if(sigpending(&pend_mask) < 0){
		perror("sigending\n");
	}

	if(sigismember(&pend_mask, SIGINT) < 0){
		printf("SIGINT is pending\n");
	}

	if(sigprocmask(SIG_SETMASK, &old_set, NULL) < 0){
		perror("sigprocmask\n");
	}
	
	printf("\nSIGINT unblock\n");
	sleep(5);
	return 0;
}
