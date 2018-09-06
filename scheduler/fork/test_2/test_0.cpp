#include<sys/sem.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>

static int sem_id;
union semun{
	int				val;
	struct semid_ds	*buf;
	unsigned short	*array;
	struct seminfo	*__buff;
};
/*
 *	对信号量的值, 进行设置
 * */
bool set_semvalue(){
	union semun sem_union;

	sem_union.val = 1;
	if(semctl(sem_id, 0, SETVAL, sem_union) == -1){
		fprintf(stderr, "semctl() failure errno = %d str_err = %s\n", errno, strerror(errno));
		return false;
	}
	return true;
}

/*
 *	移除信号量
 * */
bool del_semvalue(){
	union semun sem_union;
	
	if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1){
		fprintf(stderr, "semctl() failure errno = %d str_err = %s\n", errno, strerror(errno));
		return false;
	}
	return true;
}

/*
 *	对信号量进行-1操作
 * */
bool semaphore_p(){
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1){
		fprintf(stderr, "semop() failure errno = %d str_err = %s\n", errno, strerror(errno));
		return false;
	}
	return true;
}

/*
 *	对信号量进行+1操作
 * */
bool semaphore_v(){
	struct sembuf sem_b;
	
	sem_b.sem_num = 0;
	sem_b.sem_op  = 1;
	sem_b.sem_flg = SEM_UNDO;
	if(semop(sem_id, &sem_b, 1) == -1){
		fprintf(stderr, "semop() failure %d, str_err = %d\n", errno, strerror(errno));
		return false;
	}

	return true;
}

int main(int argc, char *argv[]){

	char op_char = 'X';
	int pause_time = -1;

	sem_id = semget((key_t)1237, 1, 0666 | IPC_CREAT);
	if(sem_id == -1){
		printf("sesget() failure errno = %d, str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if(argc > 1){
		if(!set_semvalue()){
			fprintf(stderr, "sem_semvalue() failure errno = %d str_err = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		op_char = 'O';
		sleep(3);
	}

	for(int i = 0; i < 10; i++){
		if(!semaphore_p()){
			exit(EXIT_FAILURE);
		}
		printf(" enter: %c", op_char);
		fflush(stdout);
		pause_time = rand() % 3;
		sleep(pause_time);
		printf(" exit:  %c", op_char);
		fflush(stdout);
		if(!semaphore_v()){
			exit(EXIT_FAILURE);
		}
		
		pause_time = rand() % 2;
		sleep(pause_time);
	}

	printf("\n%d -- finished\n", getpid());
	if(argc > 1){
		sleep(10);
		del_semvalue();
	}
	exit(EXIT_SUCCESS);
}
