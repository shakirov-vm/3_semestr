#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SHM_NAME "shm_general"
#define PROJ_ID 0xDEADBEAF
#define NUM_SEMAPHORES 9

int main() {
	
	key_t key = ftok(SHM_NAME, PROJ_ID);
	if (key == -1) {
		perror("ftok ");
		return 1;
	}

	int sem_id = semget(key, NUM_SEMAPHORES, IPC_CREAT | 0666);
	if (sem_id == -1) {
		perror("semget ");
		return 3;
	}

	int err = semctl (sem_id, 0, IPC_RMID, 0); 
	if (err == -1) {
		printf("Don't deleted!\n");
	}
}