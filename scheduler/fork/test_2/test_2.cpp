#define TEST_SZ	2048
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/shm.h>
#include<assert.h>
#include<errno.h>

struct share_use_st{
	int written_by_you;
	char some_text[TEST_SZ];
};

static int shm_id;
int main(){
		shm_id = shmget((key_t)1234, sizeof(struct share_use_st), 0666 | IPC_CREAT);
		if(shm_id == -1){
			fprintf(stderr, "shmget() failure errno = %d str_err = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		void *share_memory = shmat(shm_id, NULL, 0);
		if(share_memory == (void*)-1){
			fprintf(stderr, "shmat() failure errno = %d stderr = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		printf("Memory attached at %X\n", share_memory);
		struct share_use_st *share_struct = (struct share_use_st*)share_memory;
		share_struct->written_by_you = 0;
		while(true){
			if(share_struct->written_by_you){
				printf("You wrote: %s", share_struct->some_text);
				sleep(rand()%4);
				share_struct->written_by_you = 0;
				if(strncmp(share_struct->some_text, "end", 3) == 0){
					break;
				}
			}
		}

		if(shmdt(share_memory) == -1){
			fprintf(stderr, "shmdt() failure errno = %d str_err = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if(shmctl(shm_id, IPC_RMID, 0) == -1){
			fprintf(stderr, "shmctl() failure errno = %d str_err = %s\n", errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
		
		exit(EXIT_SUCCESS);
}
