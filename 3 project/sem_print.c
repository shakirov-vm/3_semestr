#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SHM_SIZE 256
#define SHM_NAME "shm_general"
#define PROJ_ID 0xDEADBEAF
#define NUM_SEMAPHORES 9

enum {

	ONE_WRITER = 0,
	ONE_READER = 1,
	WRITER_CONNECT = 2,
	READER_CONNECT = 3,
	SUMM_READ_CONNECT = 4,
	SUMM_WRIT_CONNECT = 5,
	SUMM_CONNECT = 6,
	EMPTY = 7,
	FULL = 8
};

int get_semaphore(int sem_id, int semnum) {

	int value = semctl(sem_id, semnum, GETVAL);

	return value;
}

void print_sem(int sem_id) {

	printf("ONE_WRITER = %d\n", get_semaphore(sem_id, ONE_WRITER));
	printf("ONE_READER = %d\n", get_semaphore(sem_id, ONE_READER));
	printf("WRITER_CONNECT = %d\n", get_semaphore(sem_id, WRITER_CONNECT));
	printf("READER_CONNECT = %d\n", get_semaphore(sem_id, READER_CONNECT));
	printf("SUMM_READ_CONNECT = %d\n", get_semaphore(sem_id, SUMM_READ_CONNECT));
	printf("SUMM_WRIT_CONNECT = %d\n", get_semaphore(sem_id, SUMM_WRIT_CONNECT));
	printf("SUMM_CONNECT = %d\n", get_semaphore(sem_id, SUMM_CONNECT));
	printf("EMPTY = %d\n", get_semaphore(sem_id, EMPTY));
	printf("FULL = %d\n", get_semaphore(sem_id, FULL));
}

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

	print_sem(sem_id);
}