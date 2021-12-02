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
#define DATA_NAME argv[1]
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

	one_writer_go_in[0].sem_num = ONE_WRITER;
	one_writer_go_in[0].sem_op = 0;
	one_writer_go_in[0].sem_flg = 0;

	one_writer_go_in[1].sem_num = ONE_WRITER;
	one_writer_go_in[1].sem_op = 1;
	one_writer_go_in[1].sem_flg = SEM_UNDO;

	err = semop(sem_id, one_writer_go_in, 2); // На этом месте отсекаются все writer'ы кроме одного
	if (err == -1) {
		perror("one writer ");
		return 4;
	}

	struct sembuf check_connect[3]; 

	check_connect[0].sem_num = SUMM_WRIT_CONNECT;
	check_connect[0].sem_op = 1;
	check_connect[0].sem_flg = 0;

	check_connect[1].sem_num = SUMM_CONNECT;
	check_connect[1].sem_op = 1;
	check_connect[1].sem_flg = 0;

	check_connect[2].sem_num = WRITER_CONNECT;
	check_connect[2].sem_op = 1;
	check_connect[2].sem_flg = SEM_UNDO;

	err = semop(sem_id, check_connect, 3);
	if (err != -1) { 
		if (err == -1) {
			perror("wait after one reader death ");
			return 4;
		}
	}

	init_semaphore(sem_id, FULL, 0);
	init_semaphore(sem_id, EMPTY, 1);
	
	printf("We connect\n");
	char* shm_ptr = shmat (shm_id, NULL, 0);
 	if (shm_ptr == -1) {//??
 		perror("shmat ");
 		return 6;
 	}

	int data_fd = open(DATA_NAME, O_RDONLY);
	if (data_fd == -1) {
		printf("[%s]\n", DATA_NAME);
		perror("data open ");
		return 5;
	}

	char data_buf[SHM_SIZE + sizeof(int)];
	int readed, writed;

	struct sembuf wait_connect[2];

	wait_connect[0].sem_num = READER_CONNECT;
	wait_connect[0].sem_op = -1;
	wait_connect[0].sem_flg = 0;

	wait_connect[1].sem_num = READER_CONNECT;
	wait_connect[1].sem_op = 1;
	wait_connect[1].sem_flg = 0;

	err = semop(sem_id, wait_connect, 2);
	if (err != -1) { 
		if (err == -1) {
			perror("wait connect : ");
			return 4;
		}
	}
printf("Reader connect, %d\n", get_semaphore(sem_id, READER_CONNECT));
	struct sembuf switch_controller[5];

	int read_connect_sem = get_semaphore(sem_id, SUMM_READ_CONNECT);
print_sem(sem_id);
printf("\n\nIt before while\n\n");
	while(1) { //Может стоит записывать номер итерации?

		switch_controller[0].sem_num = SUMM_CONNECT;
		switch_controller[0].sem_op = -read_connect_sem * 2;
		switch_controller[0].sem_flg = IPC_NOWAIT;

		switch_controller[1].sem_num = SUMM_CONNECT;
		switch_controller[1].sem_op = read_connect_sem * 2;
		switch_controller[1].sem_flg = 0;

		switch_controller[2].sem_num = READER_CONNECT;
		switch_controller[2].sem_op = -1;
		switch_controller[2].sem_flg = IPC_NOWAIT;

		switch_controller[3].sem_num = READER_CONNECT;
		switch_controller[3].sem_op = 1;
		switch_controller[3].sem_flg = 0;

		switch_controller[4].sem_num = EMPTY;
		switch_controller[4].sem_op = -1;
		switch_controller[4].sem_flg = 0;

		err = semop(sem_id, switch_controller, 5);
		if (err == -1) { // Если заходим - значит всё-таки отвалился
			perror("writer before read from file - reader disconnect ");
			return 4;
		}

		readed = read(data_fd, data_buf + sizeof(int), SHM_SIZE); 
		if (readed == -1) {
			perror("read from data ");
			return 7;
		}

//exit(100);

		*((int*) shm_ptr) = readed;
		strncpy(shm_ptr + 4, data_buf + 4, SHM_SIZE); // Тут перезаписываем массив, нужно аккуратно

		if (readed != 0) printf("[%s]\nnumber - %d\n", data_buf + 4, *((int*) shm_ptr) ); //Ещё можно выводить просто количество считанного

		switch_controller[0].sem_num = SUMM_CONNECT;
		switch_controller[0].sem_op = -read_connect_sem * 2;
		switch_controller[0].sem_flg = IPC_NOWAIT;

		switch_controller[1].sem_num = SUMM_CONNECT;
		switch_controller[1].sem_op = read_connect_sem * 2;
		switch_controller[1].sem_flg = 0;

		switch_controller[2].sem_num = READER_CONNECT;
		switch_controller[2].sem_op = -1;
		switch_controller[2].sem_flg = IPC_NOWAIT;

		switch_controller[3].sem_num = READER_CONNECT;
		switch_controller[3].sem_op = 1;
		switch_controller[3].sem_flg = 0;

		switch_controller[4].sem_num = FULL;
		switch_controller[4].sem_op = 1;
		switch_controller[4].sem_flg = 0;

		err = semop(sem_id, switch_controller, 5);
		if (err == -1) {
			perror("writer after read from file - reader disconnect ");
			return 4;
		}

		if (readed == 0) break;
	}

	err = shmdt(shm_ptr);
	if (err == -1) {
		perror("shmdt write ");
		return 6;
	}

	close(data_fd);
	
	// Возможно где-то стоит удалить семафоры
	// В принципе можно Write_connect не убирать, он автоматически освободится
}