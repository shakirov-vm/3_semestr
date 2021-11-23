
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int printer(size_t biggest);

struct msgbuf {
	long mtype;
};

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

	if (num_end != NULL && *num_end) {
	printf("Incorrect enter\n");
	return 4;
	}
	
	if (biggest <= 0) {
	printf("Your number less then zero or not number\n");
	return 1;
	}
////////////////////////////////////////////////////////
	printf("You enter %ld\n", biggest);

	int returned = printer(biggest);
	return returned;
}

int printer(size_t biggest) {

	int id_msg = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
	if (id_msg == -1) {
		perror("id_msg is -1\n");
		return 7;
	}

	struct msgbuf buffer;
	struct msgbuf answer;
	buffer.mtype = -1;
	answer.mtype = -1;
	long process_num = -1;
	pid_t pid = 1;
	int err = 0;
						
	for (process_num = 1; process_num < biggest + 1; process_num++) {
		if (pid > 0) {
			pid = fork();
		}

		if (pid == 0) {
			break;
		}

		if (pid < 0) {
			perror("Error with fork ");
			return 5;
		}
	}

	if (pid > 0) {

		buffer.mtype = 1; //Родительский процесс отправляет тип первого процесса

		err = msgsnd(id_msg, &buffer, sizeof(struct msgbuf) - sizeof(long), 0);
		if (err == -1) {
			perror("We don't send first message\n");
			return 9;
		}

		for (int j = biggest; j < biggest * 2; j++) {

			err = msgrcv(id_msg, &buffer, sizeof(struct msgbuf) - sizeof(long), j + 1, 0);
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

	if (pid == 0) {

		err = msgrcv(id_msg, &answer, sizeof(struct msgbuf) - sizeof(long), process_num, 0);
		if (err == -1) {
			perror("msgrcv ");
			printf("We don't recieve in %ld\n", process_num);
		}

		printf("%ld\n", process_num);

		buffer.mtype = process_num + 1;

		err = msgsnd(id_msg, &buffer, sizeof(struct msgbuf) - sizeof(long), 0);
		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %ld message\n", process_num); 
		}

		buffer.mtype = biggest + process_num + 1;

		err = msgsnd(id_msg, &buffer, sizeof(struct msgbuf) - sizeof(long), 0);
		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %ld message\n", process_num); 
		}
	}

	return 0;
}