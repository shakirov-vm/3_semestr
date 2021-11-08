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

	struct msgbuf buffer;
	int err = 0;
	long i = 1;
					
	for (i = 1; i < biggest + 1; i++) {
		if (pid > 0) {
			pid = fork();
		}

		if (pid == 0) {
			buffer.mtype = i + 1;
			break;
		}

	}

	if (pid > 0) {

		buffer.mtype = 1; //Родительский процесс отправляет тип первого процесса
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);

		for (int j = biggest; j < biggest * 2; j++) {

			err = msgrcv(id_msg, &buffer, sizeof(int), j + 1, 0);
		}

		err = msgctl(id_msg, IPC_RMID, NULL);

		printf("Rm msg\n");
	}

	struct msgbuf answer;

	if (pid == 0) {
		long tmp = i;
printf("in %d i is %ld, tmp is %ld\n", getpid(), i, tmp);
		err = msgrcv(id_msg, &answer, sizeof(int), tmp, 0);
		i = tmp;
printf("in %d i is %ld, tmp is %ld\n", getpid(), i, tmp);

		printf("%ld\n", answer.mtype);

		err = msgsnd(id_msg, &buffer, sizeof(int), 0);

		buffer.mtype = biggest + i + 1;

		err = msgsnd(id_msg, &buffer, sizeof(int), 0);
	}

	return 0;
}