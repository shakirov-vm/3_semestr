#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
	long mtype;
	int* num_id;
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

	pid_t pid = 1;

	int id_msg = msgget(IPC_PRIVATE, 0666 || IPC_CREAT);

	struct msgbuf buffer;

	buffer.num_id = (int*) calloc (1, sizeof (int));

	size_t i;

	for(i = 0; i < biggest; i++) {
		if (pid > 0) {
			pid = fork();
		}

		if (pid == 0) {
			buffer.mtype = i;
			msgsnd(id_msg, &buffer, sizeof(int), NULL);
			//printf("Child %ld, pid - %d, ppid - %d\n", i, getpid(), getppid());
			break;
		}

		if (pid <= 0) {
			printf("Error with fork\n");
			return 5;
		}
	}

	int tmp = i;
	const int process_place = i;
	ssize_t errore;

	if (pid == 0) {
		//tmp += 100;
		for (tmp; tmp > 0; tmp--) { //Critical section?
			printf("We recieve messaage in %d\n", process_place);														//   ????
			errore = msgrcv(id_msg, &buffer, sizeof(int), process_place, NULL);
			if (errore == -1) printf("Err in %d\n", process_place); 

		}

		//BAD
		errore = msgrcv(id_msg, &buffer, sizeof(int), process_place, NULL);
		printf("CHILD: %d \n", process_place);


		errore = msgrcv(id_msg, &buffer, sizeof(int), process_place, NULL);

		for (i + 1; i < biggest; i++) {
			buffer.mtype = i;
			msgsnd(id_msg, &buffer, sizeof(int), NULL);
		}
	}



	return 0;
}