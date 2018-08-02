#include"locker.hpp"

/*---------------------------------------------------------------------------------------*/
/*
 *	初始化信号量, 将value初始化为1, 对其属性不设置(只在本进程中共享)
 * */
Sem::Sem():value(1){
	assert(sem_init(&m_sem, 0, value) != -1);
}

/*
 *	其他构造函数
 * */
Sem::Sem(int _value):value(_value){

}

/*
 *	销毁信号量,
 * */
Sem::~Sem(){
	sem_destroy(&m_sem);
}

/*
 * 等待信号量 -1操作, >0将阻塞
 * */
bool Sem::Wait(){
	return sem_wait(&m_sem) == 0;
}

/*
 *	增加信号量操作, +1操作
 * */
bool Sem::Post(){
	return sem_post(&m_sem) == 0;
}

/*--------------------------------------------------------------------------------------*/

/*
 *	初始化互斥锁, 是对关键代码进行保护的一种机制, 对其属性不加设置
 * */
Mutex::Mutex(){
	assert( pthread_mutex_init(&m_mutex, NULL) == 0);
}

/*
 *	销毁互斥锁
 * */
Mutex::~Mutex(){
	pthread_mutex_destroy(&m_mutex);
}

/*
 *	获取互斥锁
 * */
bool Mutex::Lock(){
	return pthread_mutex_lock(&m_mutex) == 0;
}

/*
 * 	释放互斥锁
 * */
bool Mutex::Unlock(){
	return pthread_mutex_unlock(&m_mutex) == 0;
}

/*---------------------------------------------------------------------------------------------*/

/*
 *	条件变量, 是对共享数据量的值的一种保护机制
 *		可以是信号量的一种, 也可能是一种特殊的信号量机制
 * */
Cond::Cond(){
	assert(pthread_mutex_init(&m_mutex, NULL) != 0 && pthread_cond_init(&m_cond, NULL) != 0);
}

/*
 *	销毁条件变量
 * */
Cond::~Cond(){
	pthread_mutex_destroy(&m_mutex);
	pthread_mutex_destroy(&m_mutex);

}

/*
 *	等待条件变量
 * */
bool Cond::Wait(){

	int ret = 0;
	pthread_mutex_lock(&m_mutex);
	ret = pthread_cond_wait(&m_cond, &m_mutex);
	pthread_mutex_unlock(&m_mutex);
	return ret == 0;
}

/*
 *	唤醒等待条件变量的线程
 * */
bool Cond::Signal(){
	return pthread_cond_signal(&m_cond) == 0;
}
