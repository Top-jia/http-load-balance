#include<sys/types.h>
#include<sys/sem.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<assert.h>
#include<sys/ipc.h>
#include<sys/wait.h>
static int sem_id;

union semun{
	int				val;
	struct semid_ds	*buf;
	unsigned int	*array;
	struct seminfo	*__buf;
};

int main(){
	sem_id = semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
	assert(sem_id != -1);
	
	/*
	 *	设置sem信号量的初值, 根据val的值来判读起可用的数据.
	 * */
	union semun sembuff;
	sembuff.val = 1;
	assert(semctl(sem_id, 0, SETVAL, sembuff) != -1);

	pid_t pid = fork();
	if(pid == -1){
		assert(semctl(sem_id, 0, IPC_RMID, sembuff) != -1);
		fprintf(stderr, "fork() failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	else if(pid == 0){
		printf("child try to get binary sem\n");
		struct sembuf sem_p;
		sem_p.sem_num = 0;
		sem_p.sem_op  = -1;
		sem_p.sem_flg = SEM_UNDO;
		assert(semop(sem_id, &sem_p, 1) != -1);

		printf("child get sem and would release it after 5 second\n");
		sleep(5);

		struct sembuf sem_v;
		sem_v.sem_num = 0;
		sem_v.sem_op  = 1;
		sem_v.sem_flg = SEM_UNDO;
		assert(semop(sem_id, &sem_v, 1) != -1);
		
		exit(EXIT_SUCCESS);
	}
	else{
		printf("parent try to get binary sem\n");
		struct sembuf sem_p;
		sem_p.sem_num = 0;
		sem_p.sem_op  = -1;
		sem_p.sem_flg = SEM_UNDO;
		assert(semop(sem_id, &sem_p, 1) != -1);

		printf("parent get sem and would release it after 5 second\n");
		sleep(5);
		
		struct sembuf sem_v;
		sem_v.sem_num = 0;
		sem_v.sem_op  = 1;
		sem_v.sem_flg = SEM_UNDO;
		assert(semop(sem_id, &sem_v, 1) != -1);
	}
	
	waitpid(pid, NULL, 0);
	assert(semctl(sem_id, 0, IPC_RMID, sembuff) != -1);
	return 0;
}
