#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define SHM_SIZE 256
#define SHM_NAME "shm_general"
#define PROJ_ID 0xDEADBEAF
 
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


	int sem_id = semget(key, 3, 0); // Мы не должны их создавать заново, т.к. у них предустановленные значения от writer
	if (sem_id == -1) {
		perror("semget ");
		return 3;
	}

	// 0 отсекает все кроме одного writers, 1 readers, 2 даёт доступ к памяти либо ридеру либо врайтеру

	struct sembuf only_one_remain_writer;
	struct sembuf only_one_remain_reader; 
	struct sembuf shm_memory;

	only_one_remain_reader.sem_num = 1;
	only_one_remain_reader.sem_op = -1;
	only_one_remain_reader.sem_flg = SEM_UNDO;

	err = semop(sem_id, &only_one_remain_writer, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}

	only_one_remain_writer.sem_num = 0;
	only_one_remain_writer.sem_op = -1;
	only_one_remain_writer.sem_flg = SEM_UNDO;



	err = semop(sem_id, &only_one_remain_writer, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}

	char* shm_ptr = shmat (shm_id, NULL, 0);
 	if (shm_ptr == -1) {//??
 		perror("shmat ");
 		return 6;
 	}

	char data_buf[SHM_SIZE];
	int readed, writed;
// ТУТ ПО ХОДУ НУЖНО 2 СЕМАФОРА НА ЭТО ДЕЛО
	shm_memory.sem_num = 2;
	shm_memory.sem_op = -1;
	shm_memory.sem_flg = 0;

	err = semop(sem_id, &shm_memory, 1);
	if (err == -1) {
		perror("sem only_one_remain_writer ");
		return 4;
	}

	while(1) {

		strncpy(data_fd, shm_ptr, SHM_SIZE); // Походу надо записывать в разделяемую память доп информацию о количестве записанного
		printf("%s", buf);

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

	err = shmctl(shm_id, IPC_RMID, 0);
	if (err == -1) {
		perror("shm delete ");
		return 8;
	}
}