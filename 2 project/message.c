#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

int func(size_t biggest);

struct msgbuf {
	long mtype;
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
}

int func(size_t biggest) {
	pid_t pid = 1;
	int id_msg = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
	if (id_msg == -1) {
		perror("id_msg is -1\n");
		return 7;
	}
printf("Do MSG\n");
	struct msgbuf buffer;
	buffer.mtype = -1;
	int err = 0;

	long i;
					
	for (i = 1; i < biggest + 1; i++) {
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
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);

		if (err == -1) {
			perror("We don't send first message\n");
			return 9;
		}

		pid_t pid;
		int status;
		for (int k = 0; k < biggest; k++) {
			pid = waitpid(-1, &status, 0);
			if (WIFEXITED(status)) printf("process %d is succes, status is %d\n", pid, status);
			if (!WIFEXITED(status) || status != 0) {
				printf("ALARM!\n");
				//kill(-1, SIGTERM);
			}
		}

		for (int j = biggest; j < biggest * 2; j++) {

			err = msgrcv(id_msg, &buffer, sizeof(int), j + 1, 0);

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

	struct msgbuf answer;

	if (pid == 0) {

		long tmp = i;
		err = msgrcv(id_msg, &answer, sizeof(int), i, 0);
		i = tmp; //Потому что в первом  ребёнке i портится 
	
		if (err == -1) {
			perror("msgrcv ");
			printf("We don't recieve in %ld\n", i);
		}

		printf("%ld\n", answer.mtype);
if(i == 5) exit(15);
		buffer.mtype = i + 1;
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);

		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %ld message\n", i); 
		}

		buffer.mtype = biggest + i + 1;
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);

		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %ld message\n", i); 
		}
	}

	return 0;
}