#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <math.h>
#include <signal.h>

#include "proxy.h"

// argv[1] - quantity of child, argv[2] - file name
int main(int argc, char** argv) {

	long quantity = get_num(argc, argv);

	struct parent* childs = (struct parent*) calloc (quantity, sizeof(struct parent));

	int new_pid = -1;
	int tmp_pipefd_from_parent[2] = {0};
	int tmp_pipefd_to_parent[2] = {0};
	pid_t true_ppid = getpid();

	fd_set readfds, writefds;
	int nfds = 0;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	struct child* i_am = (struct child*) calloc (1, sizeof(struct child));

	for(int i = 0; i < quantity; i++) {

		pipe(tmp_pipefd_from_parent);
		pipe(tmp_pipefd_to_parent);
		new_pid = fork();

		if (new_pid == -1) {
			perror("fork ");
			exit(1);
		}

		if (new_pid == 0) { // child

			prctl(PR_SET_PDEATHSIG, SIGKILL);
			if (true_ppid != getppid()) {
				perror("child dead ");
				exit(1);
			}

			close(tmp_pipefd_to_parent[0]); // close read
			close(tmp_pipefd_from_parent[1]); // close write

			i_am->num = i;
			i_am->ppid = true_ppid;
			//i_am->capacity = (long) pow(3, quantity - i + 4);
			i_am->capacity = 256;
			i_am->buf = (char*) calloc (i_am[i].capacity, sizeof(char)); 
			i_am->pipefd[0] = tmp_pipefd_from_parent[0];
			i_am->pipefd[1] = tmp_pipefd_to_parent[1];

			free(childs); //Must we do this?

			break;
		}

		// parent

		close(tmp_pipefd_to_parent[1]); // close write
		close(tmp_pipefd_from_parent[0]); // close read
//printf("i - %d\n", i);
		childs[i].num = i;
		childs[i].pid = new_pid;
		childs[i].capacity = (long) pow(3, quantity - i + 4);
	//	childs[i].capacity = 256;
		childs[i].buf = (char*) calloc (childs[i].capacity, sizeof(char));
		if (i != 0) childs[i - 1].pipefd[0] = tmp_pipefd_to_parent[0];
		childs[i].pipefd[1] = tmp_pipefd_from_parent[1];
// Is it normal?
		fcntl(childs[i].pipefd[0], F_SETFL, O_RDONLY | O_NONBLOCK);
		fcntl(childs[i].pipefd[1], F_SETFL, O_WRONLY | O_NONBLOCK);

		FD_SET(childs[i].pipefd[0], &readfds);
		FD_SET(childs[i].pipefd[1], &writefds);

		if (childs[i].pipefd[0] > nfds) nfds = childs[i].pipefd[0];
		if (childs[i].pipefd[1] > nfds) nfds = childs[i].pipefd[1];
	}

	// child

	if (new_pid == 0) {
	
		if (i_am->num == 0) {

			i_am->pipefd[0] = open(argv[2], O_RDONLY); // Maybe need be more carefull
			if (i_am->pipefd[0] == -1) {
				perror("open file ");
				exit(1);
			}

			//int err_rd = read(i_am->pipefd[0], i_am->buf, i_am->capacity);
			//printf("Reader: %d - [%s]\n", err_rd, i_am->buf);
		}

		if (i_am->num == quantity - 1) {
printf("\nIt is in %d, quantity - %d\n", i_am->num, quantity);
			i_am->pipefd[1] = 1; //stdout
		}

		while(1) {
//printf("In %d child\n", i_am->num);
			int readed = read(i_am->pipefd[0], i_am->buf, i_am->capacity);
printf("in %d read %d\n", i_am->num, readed);
			if (readed == -1) {
				perror("read in child ");
				exit(1);
			}
//printf("In %d child we read %d\n", i_am->num, readed);
			int writed = write(i_am->pipefd[1], i_am->buf, readed);
			if (writed != readed) {
				printf("We write less than read\n");
			}
printf("In %d write %d\n", i_am->num, writed);
//printf("writed in %d : [%s]\n", i_am->num, i_am->buf);
			if (readed == 0) break;
		}

		if (i_am->num != 0) close(i_am->pipefd[0]);
		if (i_am->num != quantity) close(i_am->pipefd[1]);

printf("Child %d end\n", i_am->num);
	}

	else { // parent

		fd_set ready_read;
		fd_set ready_write;
		int ready_fds;
		int counter;
		int readed, writed;

		while(1) {

			counter = 0;

			ready_read = readfds;
			ready_write = writefds;

			ready_fds = select(nfds, &ready_read, &ready_write, NULL, NULL); // Is must timeout be NULL?
sleep(1);
printf("READY_FDS - %d\n", ready_fds);
// Это скорее всего не работает, так как буферы разных размеров, будет лочиться
			for(int i = 0; i < quantity; i++) {
printf("i - %d, size - %d\n", i, childs[i].size);
//printf("In for childs[i].size - %d, i - %d\n", childs[i].size, i);
				if (childs[i].size > 0) { // Если есть, что записывать
printf("i - %d, size - %d\n", i, childs[i].size);
					if (FD_ISSET(childs[i].pipefd[1], &ready_write)) {
					
						writed = write(childs[i].pipefd[1], childs[i].buf, childs[i].size);
printf("in parent we write [%s], %d in %d", childs[i].buf, writed, childs[i].size);
						//printf("Writed is %d in %d\n", writed, childs[i].num);
						childs[i].size -= writed;
						//А если не записали полностью?
					}					
				}

				if (childs[i].size == 0) {
				
					if (FD_ISSET(childs[i].pipefd[0], &ready_read)) {
printf("i - %d, size - %d\n", i, childs[i].size);
						readed = read(childs[i].pipefd[0], childs[i].buf, childs[i].capacity);
printf("in parent we read [%s], %d in %d", childs[i].buf, readed, childs[i].size);
						childs[i].size = readed;		
						if (readed == 0) {
							printf("\nREALLY 0, %d\n", childs[i].num);
							FD_CLR(childs[i].pipefd[0], &readfds);
							close(childs[i].pipefd[0]);
							writed = write(childs[i].pipefd[1], childs[i].buf, childs[i].size);
							FD_CLR(childs[i].pipefd[1], &writefds);
							close(childs[i].pipefd[1]);
							printf("close all in %d\n", childs[i].num);

						}
						//printf("taki readed - %d, i - %d, quantity - %d\n", readed, i, childs[i].num);
					}
				}
			}
		}
	}

	//if (i_am->num > -1) free(childs);
	//if (my_num != -1) free(i_am);
}

long get_num(int argc, char** argv) {

	if (argc != 3) {
		printf("Need 3 args, you enter %d\n", argc);
		return -1;
	}

	char* num_end;
	long biggest = strtol(argv[1], &num_end, 0);    //x86
	
	if (errno == ERANGE) {
		printf("Warning: your quantityber bigger then long\n");
		return -2;
	}

	if (num_end != NULL && *num_end) {
		printf("Incorrect enter\n");
		return -3;
	}
	
	if (biggest <= 0) {
		printf("Your quantityber less then zero or not quantityber\n");
		return -4;
	}

	return biggest;
}