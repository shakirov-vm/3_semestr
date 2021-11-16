#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

struct msgbuf {
	long mtype;
};

int printer(size_t biggest);

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
	return 1;
	}

	if (num_end != NULL && *num_end) {
	printf("Incorrect enter\n");
	return 1;
	}
	
	if (biggest <= 0) {
	printf("Your number less then zero or not number\n");
	return 1;
	}
////////////////////////////////////////////////////////
	printf("You enter %ld\n", biggest);

	return printer(biggest);
}

int printer(size_t biggest) {
	pid_t pid = 1;
	int id_msg = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
	if (id_msg == -1) {
		perror("id_msg is -1\n");
		return 2;
	}

	struct msgbuf buffer;
	buffer.mtype = -1;
	int err = 0;

	long i;
		
	pid_t* pids = (pid_t*) calloc (biggest, sizeof(pid_t));			
	for (i = 1; i < biggest + 1; i++) {
		if (pid > 0) {
			pids[i - 1] = fork();
			pid = pids[i - 1];
		}

		if (pid == 0) {
			break;
		}

		if (pid < 0) {
			perror("Error with fork ");
			return 3;
		}
	}

	if (pid > 0) {

		//for(int t = 0; t < biggest; t++) printf("%d ", pids[t]);

		buffer.mtype = 1; //Родительский процесс отправляет тип первого процесса
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);

		if (err == -1) {
			perror("We don't send first message\n");
			return 4;
		}

		pid_t pid_p;
		int status;

		for (int k = 0; k < biggest; k++) {
			pid_p = waitpid(-1, &status, 0);
			//if (WIFEXITED(status)) printf("process %d is succes, status is %d\n", pid, status);
			if (!WIFEXITED(status) || status != 0) {
				printf("\nALARM!, %d process is bad\n", pid_p);

				for (int i_pid = 0; i < biggest; i++) {
					err = kill(pids[i_pid], SIGTERM);
					if (err == -1) {
						printf("Can't kill %d\n", pids[i_pid]);
						perror("kill ");
					}
				}
				printf("\nkill all\n");
				break;
				//kill(-1, SIGTERM); Don't do it. It has bad end
			}
		}

		err = msgctl(id_msg, IPC_RMID, NULL);
		if (err == -1) {
			perror("msgctl() ");	
			return 4;
		}
		free(pids);
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
			return 4;
		}

		printf("%ld\n", answer.mtype);
//if(i == 5) exit(15);
		buffer.mtype = i + 1;
		err = msgsnd(id_msg, &buffer, sizeof(int), 0);
//printf("I AM ALIVE\n");
		if (err == -1) {
			perror("msgsnd ");
			printf("We don't send %ld message\n", i); 
			return 4;
		}
	}

	return 0;
}