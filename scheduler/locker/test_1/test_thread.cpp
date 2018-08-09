#include"locker.hpp"
#include<iostream>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string>

#define 	BUFF_SIZE 127

char msg[BUFF_SIZE];
Sem sem(0);

Mutex mutex;
void* thread_funtion(void *arg){
	sem.Wait();
	while(strncmp(msg, "end", 3) != 0){
		printf("this is sub_thread %s\n", (void *)arg);
		printf("this is sub_thread msg = %s\n", msg);
		memset(msg, '\0', BUFF_SIZE);
		sem.Wait();
	}

	pthread_exit((void*)"sub_thread end");
}

int main(){
	pthread_t one;
	int res = pthread_create(&one, NULL, thread_funtion, (void*)"hello sub");
	if(res != 0){
		std::cout << res << " pthread_create return error " << std::endl;
		exit(0);
	}

	memset(msg, '\0', BUFF_SIZE);
	while(strncmp(msg, "end", 3) != 0){
		printf("Input data!\n");
		fgets(msg, BUFF_SIZE, stdin);
		sem.Post();
	}

	void *reval;
	pthread_join(one, &reval);
	std::cout << "sub_thread: " << std::string((char*)reval) << std::endl;

	return 0;
}
