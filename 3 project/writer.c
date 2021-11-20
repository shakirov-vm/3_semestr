#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define SHM_SIZE 256
#define SHM_NAME "shm_general"
#define PROJ_ID 0xDEADBEAF
#define DATA_NAME argv[2]

void init_semaphore(int sem_id, int semnum, int initval) {
	union semun semopts;
	semopts.val = initval;
	semctl(sem_id, semnum, SETVAL, semopts);
}

int main(int argc, char** argv) {

	int err = 0;

	key_t key = ftok(SHM_NAME, PROJ_ID);
	if (key == -1) {
		perror("ftok ");
		return 1;
	}

	int shm_id = shmget(key, SHM_SIZE, IPC_CREAT);
	if (shm_id == -1) {
		perror("shmget ");
		return 2;
	}

	int sem_id = semget(key, 3, IPC_CREAT);
	if (sem_id == -1) {
		perror("semget ");
		return 3;
	}

	init_semaphore(sem_id, 0, 1);
	init_semaphore(sem_id, 1, 0);
	init_semaphore(sem_id, 2, 1);

// 0 отсекает все кроме одного writers, 1 readers, 2 даёт доступ к памяти либо ридеру либо врайтеру

	struct sembuf only_one_remain_writer;
	struct sembuf only_one_remain_reader; 
	struct sembuf shm_memory;

	only_one_remain_writer.sem_num = 0;
	only_one_remain_writer.sem_op = -1;
	only_one_remain_writer.sem_flg = SEM_UNDO;

	shm_memory.sem_num = 2;
	shm_memory.sem_op = -1;
	shm_memory.sem_flg = 0;

	err = semop(sem_id, &only_one_remain_writer, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}

	err = semop(sem_id, &shm_memory, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}

	char* shm_ptr = shmat (shm_id, NULL, 0);
 	if (shm_ptr == -1) {//??
 		perror("shmat ");
 		return 6;
 	}

 	only_one_remain_reader.sem_num = 1;
	only_one_remain_reader.sem_op =  1;
	only_one_remain_reader.sem_flg = SEM_UNDO;

	err = semop(sem_id, &only_one_remain_writer, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}

	int data_fd = open(DATA_NAME, RD_ONLY);
	if (data_fd == -1) {
		perror("data open ");
		return 5;
	}

	char data_buf[SHM_SIZE];
	int readed, writed;

	while(1) {
//Сначала ставим семафор на то, что запрещаем читать
		readed = read(data_fd, data_buf, SHM_SIZE); 
		if (readed == -1) {
			perror("read from data ");
			return 7;
		}

		strncpy(shm_ptr, data_fd, readed);
		if (readed != 0) printf("%s", buf); //Ещё можно выводить просто количество считанного
//Потом разрешаем читать
//Потом запрещаем писать?
		shm_memory.sem_num = 2;
		shm_memory.sem_op =  1;
		shm_memory.sem_flg = 0;

		err = semop(sem_id, &shm_memory, 1);
		if (err == -1) {
			perror("sem only_one_remain_writer ");
			return 4;
		}

		if (readed != SHM_SIZE && readed != 0) continue;
		if (readed == 0) break;
	}

	only_one_remain_writer.sem_num = 0;
	only_one_remain_writer.sem_op = 1;
	only_one_remain_writer.sem_flg = SEM_UNDO;

	err = semop(sem_id, &only_one_remain_writer, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}


	err = shmdt(shm_ptr);
	if (err == -1) {
		perror("shmdt write ");
		return 6;
	}

	close(data_fd);

	err = shmctl(shm_id, IPC_RMID, 0);
	if (err == -1) {
		perror("shm delete ");
		return 8;
	}
}