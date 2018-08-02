#ifndef LOCKER_HPP
#define LOCKER_HPP

#include<pthread.h>
#include<semaphore.h>
#include<assert.h>

/*信号量类*/
class Sem{

		sem_t m_sem;
		unsigned int value;
	public:
		Sem();
		Sem(int _value);
		~Sem();
		bool Wait();
		bool Post();
};

/*互斥锁类*/
class Mutex{
		pthread_mutex_t m_mutex;
	public:
		Mutex();
		~Mutex();
		bool Lock();
		bool Unlock();
};

/*定义条件变量*/
class Cond{
		pthread_mutex_t m_mutex;
		pthread_cond_t m_cond;
	public:
		Cond();
		~Cond();
		bool Wait();
		bool Signal();
};

#endif


