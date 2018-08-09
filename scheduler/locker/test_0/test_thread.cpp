#include"locker.hpp"
#include<iostream>
#include<string>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#define BUFF_SIZE 127

Mutex mutex;
void* thread_funtion(void *arg){
	printf("thread_funtion = %s\n", (char*)arg);
	mutex.Lock();
	memset(arg, '\0', strlen((char*)arg));
	strcpy((char *)arg, "host thread");
	mutex.Unlock();
	pthread_exit((void*)"this is jobs");
}

int main(){
	pthread_t reader;
	char str[BUFF_SIZE];
	memset(str, '\0', BUFF_SIZE);
	strcpy(str, "this is thread_funtion ");
	int res = pthread_create(&reader, NULL, thread_funtion, (void*)str);
	if(res != 0){
		printf("Thread create failed");
		exit(EXIT_FAILURE);
	}
	sleep(2);
	mutex.Lock();
	memset(str, '\0', BUFF_SIZE);
	strcpy(str, "wnag");
	printf("%s\n", str);
	mutex.Unlock();

	void *reval;
	pthread_join(reader, &reval);
	printf("return_val = %s\n", reval);
	return 0;
}
