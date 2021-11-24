#include <errno.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define SHM_SIZE 256
#define SHM_NAME "shm_general"
#define PROJ_ID 0xDEADBEAF
#define NUM_SEMAPHORES 6

enum {

	ONE_WRITER = 0,
	ONE_READER = 1,
	WRITER_CONNECT = 2,
	READER_CONNECT = 3,
	EMPTY = 4,
	FULL = 5,
};

int get_semaphore(int sem_id, int semnum) {

	int value = semctl(sem_id, semnum, GETVAL);

	return value;
}

int main(int argc, char** argv) {
	if (argc != 1) {
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

	int sem_id = semget(key, NUM_SEMAPHORES, IPC_CREAT | 0666); // Что с установкой значений? 
	if (sem_id == -1) {
		perror("semget ");
		return 3;
	}

	struct sembuf one_reader_go_in[2];

	//one_reader_go_in[0] = {ONE_READER, 0, 0};
	one_reader_go_in[0].sem_num = ONE_READER;
	one_reader_go_in[0].sem_op = 0;
	one_reader_go_in[0].sem_flg = 0;
	//one_reader_go_in[1] = {ONE_READER, 1, SEM_UNDO};
	one_reader_go_in[1].sem_num = ONE_READER;
	one_reader_go_in[1].sem_op = 1;
	one_reader_go_in[1].sem_flg = SEM_UNDO;

	err = semop(sem_id, one_reader_go_in, 2); // На этом месте отсекаются все writer'ы кроме одного
	if (err == -1) {
		perror("one reader ");
		return 4;
	}

	struct sembuf check_connect[2]; 
//Точно ли здесь 2, а не 1?
	//check_connect[0] = {READER_CONNECT, -1, IPC_NOWAIT};
	check_connect[0].sem_num = READER_CONNECT;
	check_connect[0].sem_op = -1;
	check_connect[0].sem_flg = IPC_NOWAIT;
	//check_connect[1] = {READER_CONNECT, 1, 0};
	check_connect[1].sem_num = READER_CONNECT;
	check_connect[1].sem_op = 1;
	check_connect[1].sem_flg = 0;

	err = semop(sem_id, check_connect, 2); // Проверяет, что второй процесс вошёл
	if (err != -1) { // Сюда входит, если изначально READER_CONNECT == 2
printf("We have check of old\n");
		struct sembuf wait_free_writer[1];
		//wait_free_writer[0] = {READER_CONNECT, 0, 0}; // Ждём, пока завершится writer, войдёт новый и сконнектится к этому
		wait_free_writer[0].sem_num = READER_CONNECT;
		wait_free_writer[0].sem_op = 0;
		wait_free_writer[0].sem_flg = 0;

		err = semop(sem_id, wait_free_writer, 1); 
		if (err == -1) {
			perror("wait after one reader death ");
			return 4;
		}
	}
printf("We must go there\n");
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
		perror("connect from reader ");
		return 4;
	}
printf("We was ++\n");
printf("WR_CN - %d, RD_CN - %d\n", get_semaphore(sem_id, WRITER_CONNECT), get_semaphore(sem_id, READER_CONNECT)); 
	//check_connect[0] = {WRITER_CONNECT, -2, 0}; // Тут не NO_WAIT, т.к. нам нужно, что бы оба вошли это синхронизация
	check_connect[0].sem_num = WRITER_CONNECT;
	check_connect[0].sem_op = -2;
	check_connect[0].sem_flg = 0;
	//check_connect[1] = {WRITER_CONNECT, 2, 0};
	check_connect[1].sem_num = WRITER_CONNECT;
	check_connect[1].sem_op = 2;
	check_connect[1].sem_flg = 0;

// Нужно добавить проверку
	err = semop(sem_id, check_connect, 2); // Проверяет, что второй процесс вошёл
	if (err == -1) {
		perror("wait writer error ");
		return 4;
	}
	
	printf("We connect\n");
	char* shm_ptr = shmat (shm_id, NULL, 0);
 	if (shm_ptr == -1) {//??
 		perror("shmat ");
 		return 6;
 	}
sleep(20);
	char data_buf[SHM_SIZE];
	int readed, writed = 1;

	/*check_connect[0] = {WRITER_CONNECT, -2, IPC_NOWAIT}; // Это уже проверка на вшивость
	check_connect[1] = {WRITER_CONNECT, 2, 0};

	err = semop(sem_id, check_connect, 2);
	if (err == -1) { // Если заходим - значит всё-таки отвалился
		perror("reader before while in writer ");
		return 4;
	}*/

	struct sembuf switch_controller[3]; 

	while(1) {

		//switch_controller[0] = {WRITER_CONNECT, -2, IPC_NOWAIT};
		switch_controller[0].sem_num = WRITER_CONNECT;
		switch_controller[0].sem_op = -2;
		switch_controller[0].sem_flg = IPC_NOWAIT;
		//switch_controller[1] = {WRITER_CONNECT, 2, 0};
		switch_controller[1].sem_num = WRITER_CONNECT;
		switch_controller[1].sem_op = 2;
		switch_controller[1].sem_flg = 0;
		//switch_controller[2] = {FULL, -1, 0};
		switch_controller[2].sem_num = FULL;
		switch_controller[2].sem_op = -1;
		switch_controller[2].sem_flg = 0;

		err = semop(sem_id, switch_controller, 3);
		if (err == -1) { // Если заходим - значит всё-таки отвалился
			writed = *((int*) shm_ptr);
			if (writed != 0) {
				perror("reader before read from shared memory - writer disconnect ");
				return 4;
			}
		}

		writed = *((int*) shm_ptr);

		shm_ptr += 4; 
		strncpy(data_buf, shm_ptr, writed);

		int tmp = writed;
		for(int i = writed; i != SHM_SIZE; i++) shm_ptr[i] = 0;

		if (writed > 0) printf("%s", shm_ptr);
		shm_ptr -= 4;

		//switch_controller[0] = {WRITER_CONNECT, -2, IPC_NOWAIT};
		switch_controller[0].sem_num = WRITER_CONNECT;
		switch_controller[0].sem_op = -2;
		switch_controller[0].sem_flg = IPC_NOWAIT;
		//switch_controller[1] = {WRITER_CONNECT, 2, 0};
		switch_controller[1].sem_num = WRITER_CONNECT;
		switch_controller[1].sem_op = 2;
		switch_controller[1].sem_flg = 0;
		//switch_controller[2] = {EMPTY, 1, 0};
		switch_controller[2].sem_num = EMPTY;
		switch_controller[2].sem_op = 1;
		switch_controller[2].sem_flg = 0;

		err = semop(sem_id, switch_controller, 3);
		if (err == -1 && writed != 0) { // Если заходим - значит всё-таки отвалился
			printf("writed is %d\n", writed);
			perror("reader after read from shared memory - writer disconnect ");
			return 4;
		}

		if (writed == 0) break;
	}

	err = shmdt(shm_ptr);
	if (err == -1) {
		perror("shmdt read ");
		return 6;
	}

	/*err = shmctl(shm_id, IPC_RMID, 0);
	if (err == -1) {
		perror("shm delete read ");
		return 8;
	}*/

	// Возможно где-то стоит удалить семафоры
	// В принципе можно Read_connect не убирать, он автоматически освободится
}