#include"../locker.hpp"
#include<pthread.h>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<iostream>

pthread_cond_t cond;
pthread_mutex_t mutex;

int flag = 0;

void fun(void *arg){
	pthread_mutex_unlock((pthread_mutex_t*)arg);
}

void* thread_funtion1(void *){
	/*		注册回调函数, 当在锁中被某个系统调用陷入中断(accept), 其他线程不能再等了.
	 * 当其他线程发出pthread_cancel的操作的时候, 此时, 线程立马跳出, (但是还在锁中),
	 * 锁还没有被释放, 此时, 我们注册一个回调函数, 用回调函数来处理锁的问题, 其中和
	 * pthread_cleanup_pop(0), 相对应*/
	pthread_cleanup_push(fun, &mutex);
	while(true){
		printf("thread1 is running \n");
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		printf("thread1 applied the condition\n");
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}

	pthread_cleanup_pop(0);
}

void* thread_funtion2(void *arg){
	while(true){
		printf("thread2 is running \n");
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		printf("thread2 applied the condition\n");
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
}

int main(){
	int num = 0;
	pthread_t tid1, tid2;

	printf("condition variable study!\n");
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_create(&tid1, NULL, thread_funtion1, NULL);
	pthread_create(&tid2, NULL, thread_funtion2, NULL);

	do{
		pthread_cond_signal(&cond);//给等待的线程发送信号
	}while(1);

	sleep(50);
	pthread_exit(0);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);
	return 0;
}
