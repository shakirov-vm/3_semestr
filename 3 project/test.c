#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>

#define SHM_SIZE 256
#define SHM_NAME "shm_general"
#define PROJ_ID 0xDEADBEAF
 

int main() {
	
	key_t key = ftok(SHM_NAME, PROJ_ID);
	if (key == -1) {
		perror("ftok ");
		return 1;
	}

	int sem_id = semget(key, 3, 0666 | IPC_CREAT); // Мы не должны их создавать заново, т.к. у них предустановленные значения от writer
	if (sem_id == -1) {
		if (errno == EACCES) printf("EACCESS\n");
		perror("semget ");
		return 3;
	}

	struct sembuf only_one_remain_reader[2];

	only_one_remain_reader[0].sem_num = 1;
	only_one_remain_reader[0].sem_op =  1;
	only_one_remain_reader[0].sem_flg = IPC_NOWAIT;

	only_one_remain_reader[1].sem_num = 1;
	only_one_remain_reader[1].sem_op = -1;
	only_one_remain_reader[1].sem_flg = 0;

	int err = semop(sem_id, only_one_remain_reader, 2);
	if (err == -1) {
		if (errno == EAGAIN) printf("EAGAIN\n");
		perror("check this sem - don't exist ");
		//return 4;
	}
	printf("SUCSESS END\n");
}
