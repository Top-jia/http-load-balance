#include<stdio.h>
#include<signal.h>
#include<bits/signum.h>
#include<bits/sigset.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>


/*信号处理函数*/
void sig_handler(int sig){
	int save_errno = errno;
	printf("sig = %d str_sig = %s\n", sig, strsignal(sig));
	errno = save_errno;
}

/*设置信号处理函数*/
void addsig(int signum){
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = sig_handler;
	sa.sa_flags |= SA_RESTART;
	
	sigset_t sigdest;
	sigemptyset(&sigdest);
	int err = sigprocmask(-1, NULL, &sigdest);
	if(err == 0){
		printf("before: sigdest = %x\n", sigdest);
	}
	printf("sa_sa_mask = %x\n", sa.sa_mask);
	sigfillset(&sa.sa_mask);
	printf("sa_sa_mask = %x\n", sa.sa_mask);
	
	err = sigdelset(&sa.sa_mask, signum);
	printf("-------------------------------------sa.sa_mask = %x\n", sa.sa_mask);
	//err = sigaction(signum, &sa, NULL);
	err = sigprocmask(SIG_SETMASK, &sa.sa_mask, &sigdest);
	printf("sa_sa_mask = %x sigdest = %x\n", sa.sa_mask, sigdest);

	sigemptyset(&sigdest);
	err = sigprocmask(-1, NULL, &sigdest);
	if(err == 0){
		printf("after: sigdest = %x\n", sigdest);
	}
}

int main(){
	sigset_t segdest, sigleft, sigright;
	sigemptyset(&segdest);
	sigemptyset(&sigleft);
	sigemptyset(&sigright);

	sigaddset(&sigleft, SIGALRM);
	sigaddset(&sigleft, SIGINT);

	sigaddset(&sigright, SIGALRM);
	sigaddset(&sigright, SIGQUIT);

	printf("\nSIG union\n");
	sigorset(&segdest, &sigright, &sigleft);
	if(sigismember(&segdest, SIGALRM)){
		printf("\nSIGALRM\n");
	}

	if(sigismember(&segdest, SIGINT)){
		printf("\nSIGINT\n");
	}

	if(sigismember(&segdest, SIGQUIT)){
		printf("\nSIGQUIT\n");
	}

	printf("\nSIG intersection\n");
	sigemptyset(&segdest);

	sigandset(&segdest, &sigleft, &sigright);
	if(sigismember(&segdest, SIGALRM)){
		printf("SIGALRM\n");
	}

	if(sigismember(&segdest, SIGINT)){
		printf("SIGINT\n");
	}

	if(sigismember(&segdest, SIGQUIT)){
		printf("SIGQUIT\n");
	}

	printf("------------------\n");
	sigemptyset(&segdest);
	if(sigisemptyset(&segdest)){
		printf("sigisemptyset is true\n");
	}

	//sigaddset(&segdest, SIGINT);
	sigfillset(&segdest);
	if(sigismember(&segdest, SIGINT) && sigismember(&segdest, SIGQUIT)){
		printf("sigfillset() is true\n");
	}

	sigemptyset(&segdest);
	addsig(SIGINT);
	while(1)
	{
		sleep(1);
		int err = sigprocmask(-1, NULL, &segdest);
		if(err == -1){
			fprintf(stderr, "sigprocmask() failure errno = %d str_err = %s\n", errno, strerror(errno));
		}
		if(sigismember(&segdest, SIGALRM)){
			printf("SIGALRM\n");
		}

		if(sigismember(&segdest, SIGINT)){
			printf("SIGINT\n");
		}

		if(sigismember(&segdest, SIGQUIT)){
			printf("SIGQUIT\n");
		}

		printf("segdest = %d\n", segdest);
	}
	return 0;

}
