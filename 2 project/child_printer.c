#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int func(size_t biggest);

struct msgbuf {
	long mtype;
	int  process_id;
};

int argv_handle (char* char_num);

int main(size_t argc, char** argv) {
	if (argc != 2) {
		printf("Need 2 args, you enter %ld\n", argc);
		return 1;
	}
	printf("Start\n");

	char* num_end;
	long biggest = strtol(argv[1], &num_end, 0);    //x86
	if (errno == ERANGE) {
	printf("Warning: your number bigger then long\n");
	return 3;
	}

	if (num_end != NULL && *num_end/* != ''*/) { // ?
	printf("Incorrect enter\n");
	return 4;
	}
	
	if (biggest <= 0) {
	printf("Your number less then zero or not number\n");
	return 1;
	}
////////////////////////////////////////////////////////
	printf("You enter %ld\n", biggest);

	return func(biggest);	
	//system("pause");
}

int func(size_t biggest) {
	pid_t pid = 1;
	int id_msg = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
	if (id_msg == -1) {
		perror("id_msg is -1\n");
		return 7;
	}

	struct msgbuf buffer;
	int err = 0;

	size_t i;
					///////////
	for (i = 1; i < biggest + 1; i++) {
		if (pid > 0) {
			pid = fork();
		}

		if (pid == 0) {
			buffer.mtype = i + 1;
			buffer.process_id = i + 1;
			break;
		}

		if (pid <= 0) {
			perror("Error with fork\n");
			return 5;
		}
	}

	if (pid > 0) {

		buffer.mtype = 1; //Родительский процесс отправляет тип первого процесса
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);
		//exit(10);
		if (err == -1) {
			perror("We don't send first message\n");
			return 9;
		}
		for (int j = biggest; j < biggest * 2; j++) {
//printf("We go to take at parent %d\n", j + 1);
			err = msgrcv(id_msg, &buffer, sizeof(int), j + 1, 0);
//printf("We take at parent %d\n", j + 1);
			if (err == -1) {
				perror("msgrcv() ");
				printf("We don't recieve in %d\n", j + 1);
			}
		}
		err = msgctl(id_msg, IPC_RMID, NULL);
		if (err == -1) {
			perror("msgctl() ");	
			return 8;
		}
		printf("Rm msg\n");
	}

	const int process_place = i;
	struct msgbuf answer;

	if (pid == 0) {

		//printf("Process_place - %d\n", process_place);
		err = msgrcv(id_msg, &answer, sizeof(int), process_place, 0);
		if (err == -1) {
			perror("msgrcv ");
			printf("We don't recieve in %d\n", process_place);
		}
//if (process_place == 3) exit(12);

		printf("%ld\n", answer.mtype);

		err = msgsnd(id_msg, &buffer, sizeof(int), 0);
		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %d message\n", process_place); 
		}

		buffer.mtype = biggest + i + 1;
		buffer.process_id = biggest + i + 1;
//printf("We go to send on child %d\n", process_place);
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);
//printf("We send from child %d\n", process_place);
		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %d message\n", process_place); 
		}
	}

	return 0;
}