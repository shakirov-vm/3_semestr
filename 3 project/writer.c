#include <errno.h>
#include <sys/sem.h>
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
#define DATA_NAME argv[2]
#define NUM_SEMAPHORES 6
/*
#define ONE_WRITER sem_id[0]
#define ONE_READER sem_id[1]
#define WRITER_CONNECT sem_id[2]
#define READER_CONNECT sem_id[3]
#define EMPTY sem_id[4]
#define FULL sem_id[5]
*/
enum {

	ONE_WRITER = 0,
	ONE_READER = 1,
	WRITER_CONNECT = 2,
	READER_CONNECT = 3,
	EMPTY = 4,
	FULL = 5
};

union semun {
	int val; 				
	struct semid_ds* buf; 	
	unsigned short* array; 
};

int init_semaphore(int sem_id, int semnum, int initval) {
	union semun semopts;
	semopts.val = initval;
	return semctl(sem_id, semnum, SETVAL, semopts);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Invalid args quantity\n");
		return 1;
	}

	int err = 0;

	key_t key = ftok(SHM_NAME, PROJ_ID);
	if (key == -1) {
		perror("ftok ");
		return 1;
	}

	int shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
	if (shm_id == -1) {
		perror("shmget ");
		return 2;
	}

	int sem_id = semget(key, NUM_SEMAPHORES, IPC_CREAT | 0666);
	if (sem_id == -1) {
		perror("semget ");
		return 3;
	}

	struct sembuf one_writer_go_in[2];

	//one_writer_go_in[0] = {ONE_WRITER, 0, 0};
	one_writer_go_in[0].sem_num = ONE_WRITER;
	one_writer_go_in[0].sem_op = 0;
	one_writer_go_in[0].sem_flg = 0;
	//one_writer_go_in[1] = {ONE_WRITER, 1, SEM_UNDO};
	one_writer_go_in[1].sem_num = ONE_WRITER;
	one_writer_go_in[1].sem_op = 1;
	one_writer_go_in[1].sem_flg = SEM_UNDO;

	err = semop(sem_id, one_writer_go_in, 2); // На этом месте отсекаются все writer'ы кроме одного
	if (err == -1) {
		perror("one writer ");
		return 4;
	}

	init_semaphore(sem_id, READER_CONNECT, 0);
	init_semaphore(sem_id, WRITER_CONNECT, 0);
	init_semaphore(sem_id, FULL, 0);
	init_semaphore(sem_id, EMPTY, 1);

	struct sembuf connect[2];

	// No SEM_UNDO?
	//connect[0] = {WRITER_CONNECT, 1, SEM_UNDO};
	connect[0].sem_num = WRITER_CONNECT;
	connect[0].sem_op = 1;
	connect[0].sem_flg = SEM_UNDO;
	//connect[1] = {READER_CONNECT, 1, SEM_UNDO};
	connect[1].sem_num = READER_CONNECT;
	connect[1].sem_op = 1;
	connect[1].sem_flg = SEM_UNDO;

	err = semop(sem_id, connect, 2); // Приконнекчиваем. Если второй будет идти после отвалившегося первого,
	// то он не получит второго приконнекта
	if (err == -1) {
		perror("connect from writer ");
		return 4;
	}

	struct sembuf check_connect[2]; 

	//check_connect[0] = {READER_CONNECT, -2, 0}; // Тут не NO_WAIT, т.к. нам нужно, что бы оба вошли это синхронизация
	check_connect[0].sem_num = READER_CONNECT;
	check_connect[0].sem_op = -2;
	check_connect[0].sem_flg = 0;
	//check_connect[1] = {READER_CONNECT, 2, 0};
	check_connect[1].sem_num = READER_CONNECT;
	check_connect[1].sem_op = 2;
	check_connect[1].sem_flg = 0;

	err = semop(sem_id, check_connect, 2); // Проверяет, что второй процесс вошёл
	if (err == -1) {
		perror("wait reader error ");
		return 4;
	}

	char* shm_ptr = shmat (shm_id, NULL, 0);
 	if (shm_ptr == -1) {//??
 		perror("shmat ");
 		return 6;
 	}

	int data_fd = open(DATA_NAME, O_RDONLY);
	if (data_fd == -1) {
		perror("data open ");
		return 5;
	}

	char data_buf[SHM_SIZE + sizeof(int)];
	int readed, writed;

	/*check_connect[0] = {READER_CONNECT, -2, IPC_NOWAIT}; // Это уже проверка на вшивость
	check_connect[1] = {READER_CONNECT, 2, 0};

	err = semop(sem_id, check_connect, 2);
	if (err == -1) { // Если заходим - значит всё-таки отвалился
		perror("reader before while in writer ");
		return 4;
	}*/

	struct sembuf switch_controller[3]; 

	while(1) { //Может стоит записывать номер итерации?

		//switch_controller[0] = {READER_CONNECT, -2, IPC_NOWAIT};
		switch_controller[0].sem_num = READER_CONNECT;
		switch_controller[0].sem_op = -2;
		switch_controller[0].sem_flg = IPC_NOWAIT;
		//switch_controller[1] = {READER_CONNECT, 2, 0};
		switch_controller[1].sem_num = READER_CONNECT;
		switch_controller[1].sem_op = 2;
		switch_controller[1].sem_flg = 0;
		//switch_controller[2] = {EMPTY, -1, 0};
		switch_controller[2].sem_num = EMPTY;
		switch_controller[2].sem_op = -1;
		switch_controller[2].sem_flg = 0;

		err = semop(sem_id, switch_controller, 3);
		if (err == -1) { // Если заходим - значит всё-таки отвалился
			perror("writer before read from file - reader disconnect ");
			return 4;
		}

		readed = read(data_fd, data_buf, SHM_SIZE); 
		if (readed == -1) {
			perror("read from data ");
			return 7;
		}

		/*if (readed == 0) {
			
			switch_controller[0] = {READER_CONNECT, -2, IPC_NOWAIT};
			switch_controller[1] = {READER_CONNECT, 2, 0};
			switch_controller[2] = {FULL, 1, 0};

			err = semop(sem_id, switch_controller, 3);
			if (err == -1) {
				perror("sem only_one_remain_writer ");
				return 4;
			}

			break; // ???
		}*/

		//(int) data_buf = readed; // На этой строчке записываем количество считанного
		sprintf(data_buf, readed); // BAD?
		strncpy(shm_ptr, data_buf + 4, SHM_SIZE); // Тут перезаписываем массив, нужно аккуратно
		if (readed != 0) printf("%s", data_buf + 4); //Ещё можно выводить просто количество считанного

		//switch_controller[0] = {READER_CONNECT, -2, IPC_NOWAIT};
		switch_controller[0].sem_num = READER_CONNECT;
		switch_controller[0].sem_op = -2;
		switch_controller[0].sem_flg = IPC_NOWAIT;
		//switch_controller[1] = {READER_CONNECT, 2, 0};
		switch_controller[1].sem_num = READER_CONNECT;
		switch_controller[1].sem_op = 2;
		switch_controller[1].sem_flg = 0;
		//switch_controller[2] = {FULL, 1, 0};
		switch_controller[2].sem_num = FULL;
		switch_controller[2].sem_op = 1;
		switch_controller[2].sem_flg = 0;

		err = semop(sem_id, switch_controller, 3);
		if (err == -1) {
			perror("writer after read from file - reader disconnect ");
			return 4;
		}

		//if (readed != SHM_SIZE && readed != 0) continue;
		if (readed == 0) break; // ???
	}

	err = shmdt(shm_ptr);
	if (err == -1) {
		perror("shmdt write ");
		return 6;
	}

	close(data_fd);

	err = shmctl(shm_id, IPC_RMID, 0);
	if (err == -1) {
		perror("shm delete write ");
		return 8;
	}

	// Возможно где-то стоит удалить семафоры
	// В принципе можно Write_connect не убирать, он автоматически освободится
}