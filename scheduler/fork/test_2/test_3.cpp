#include<unistd.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#define TEXT_SZ	2048
struct share_use_st{
	int written_by_you;
	char some_text[TEXT_SZ];
};

static int shm_id;

int main(){
	shm_id = shmget((key_t)1234, sizeof(struct share_use_st), 0666 | IPC_CREAT);
	if(shm_id == -1){
		fprintf(stderr, "shmget() failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	void *share_memory = shmat(shm_id, (void*)NULL, 0);
	if(share_memory == (void*)-1){
		fprintf(stderr, "shmat() failure errno = %d std_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	printf("cli Memory attached at %X\n", share_memory);
	struct share_use_st *share_struct = (struct share_use_st*)share_memory;
	
	char buffer[TEXT_SZ];
	memset(buffer, '\0', TEXT_SZ);
	while(true){
		while(share_struct->written_by_you == 1){
			sleep(1);
			printf("waiting for client:\n");
		}
		printf("Enter: some text\n");
		fgets(buffer, TEXT_SZ-1, stdin);
		
		strncpy(share_struct->some_text, buffer, TEXT_SZ-1);
		share_struct->written_by_you = 1;

		if(strncmp(buffer, "end", 3) == 0){
			break;
		}
		memset(buffer, '\0', strlen(buffer));
	}

	if(shmdt(share_memory) == -1){
		fprintf(stderr, "shmdt() failure errno = %d str_err = %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
