#include <stdio.h>
#include <pthread.h>
#include<unistd.h>
#include<stdlib.h>
void *thread_func(void *p_arg)
{
	while (1)
	{
		printf("%s\n", (char*)p_arg);
		sleep(10);
	}
}
int main(void)
{
	pthread_t t1, t2;
	pthread_create(&t1, NULL, thread_func, (void*)"Thread 1");
	pthread_create(&t2, NULL, thread_func, (void*)"Thread 2");
	sleep(1000);
	return 0;
}
