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

	int err;
	fd_set readfds, writefds;
	int nfds = 0;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);

	struct child* i_am = (struct child*) calloc (1, sizeof(struct child));

	pid_t new_pid = 0;

    for(int i = 0; i < quantity; i++) {
    	
		int tmp_pipefd_from_parent[2] = {0};
		int tmp_pipefd_to_parent[2] = {0};
       
        err = pipe (tmp_pipefd_to_parent);
        if (err == -1) {
            perror("pipe ");
            exit(1);
        }
        if (i != 0) {
            err = pipe (tmp_pipefd_from_parent);
            if (err == -1) {
                perror("pipe ");
                exit(1);
            }
        }

        new_pid = fork();

        if (new_pid == -1) {
            perror("fork ");
            exit(1);
        }

        if (new_pid > 0) { //parent

            close(tmp_pipefd_to_parent[1]);
            if (i != 0) { 
                close(tmp_pipefd_from_parent[0]);
                childs[i - 1].pipefd[1] = tmp_pipefd_from_parent[1];
            }

            childs[i].pipefd[0] = tmp_pipefd_to_parent[0];
            childs[i].capacity = (long) pow(3, quantity - i + 4);
            childs[i].buf = calloc (1, childs[i].capacity);
            if (childs[i].buf == NULL) {
                perror("childs buf calloc ");
                exit(1);
            }

            FD_SET(tmp_pipefd_to_parent[0], &readfds);

            if (nfds < tmp_pipefd_to_parent[0] + 1) nfds = tmp_pipefd_to_parent[0] + 1;
            if (i != 0 && (nfds < tmp_pipefd_from_parent[1] + 1)) nfds = tmp_pipefd_from_parent[1] + 1;
        }

        else { // child

            close(tmp_pipefd_to_parent[0]);

            if (i != 0) {
                close(tmp_pipefd_from_parent[1]); 
                i_am->pipefd[0] = tmp_pipefd_from_parent[0];
            }

            if (i == 0) i_am->pipefd[0] = open(argv[2], O_RDONLY); 

            i_am->num = i;
            i_am->pipefd[1] = tmp_pipefd_to_parent[1];
            i_am->capacity = (long) pow(3, quantity - i + 4);
            i_am->buf = calloc (1, childs[i].capacity);
            if (i_am->buf == NULL) {
                perror("i_am buf calloc ");
                exit(1);
            }

            for(int j = 0; j < i; j++) {

                close(childs[j].pipefd[0]);
                close(childs[j].pipefd[1]);
                
                free(childs[j].buf);
            }

            free(childs);

            break;
        }
    }

	if (new_pid > 0) { // parent

        childs[quantity - 1].pipefd[1] = STDOUT_FILENO; 
        FD_SET (STDOUT_FILENO, &writefds);

		for(int i = 0; i < quantity; i++) {

            fcntl(childs[i].pipefd[0], F_SETFL, O_RDONLY | O_NONBLOCK);
            fcntl(childs[i].pipefd[1], F_SETFL, O_WRONLY | O_NONBLOCK);
        }

		fd_set ready_read;
		fd_set ready_write;
		int ready_fds;
		int readed, writed;
		int finished = 0;
        int count = 0;

		while(finished != quantity) {

			ready_read = readfds;
			ready_write = writefds;

			ready_fds = select(nfds, &ready_read, &ready_write, NULL, NULL);
            count++;

			for(int i = 0; i < quantity; i++) {

                if (FD_ISSET(childs[i].pipefd[0], &ready_read)) {

                    readed = read(childs[i].pipefd[0], childs[i].buf, childs[i].capacity);
                    if (readed == -1) {
                    	perror("read ");
                    	exit(1);
                    }

                    childs[i].size = readed;

                    if (readed != 0) {

                        FD_CLR(childs[i].pipefd[0], &readfds);
                        FD_SET(childs[i].pipefd[1], &writefds);
                    }

                    else {

                        FD_CLR(childs[i].pipefd[0], &readfds);
                        close(childs[i].pipefd[0]);
                        close(childs[i].pipefd[1]);
                        free(childs[i].buf);

                        finished++;
                    }
                }

                if (FD_ISSET(childs[i].pipefd[1], &ready_write)) {

                    writed = write(childs[i].pipefd[1], childs[i].buf + childs[i].offset, childs[i].size);
                    if (writed == -1) {
                    	perror("write ");
                    	exit(1);
                    }
    
                    childs[i].offset += writed;
                    childs[i].size -= writed;

                    if (childs[i].size == 0) {

                        FD_CLR(childs[i].pipefd[1], &writefds);
                        FD_SET(childs[i].pipefd[0], &readfds);
                        childs[i].offset = 0;
                    }
                }
            }
		}
		printf("Parent end\n");
	}

	else { // child

		int readed, writed;

		while(1) {
            
			readed = read(i_am->pipefd[0], i_am->buf, i_am->capacity);
			if (readed == -1) {
				perror("read in child ");
				exit(1);
			}

			writed = write(i_am->pipefd[1], i_am->buf, readed);
			if (writed != readed) {
				printf("We write less than read\n");
			}

			if (readed == 0) break;
		}

        free(i_am);
	}
}

long get_num(int argc, char** argv) {

	if (argc != 3) {
		printf("Need 3 args, you enter %d\n", argc);
		return -1;
	}

	char* num_end;
	long biggest = strtol(argv[1], &num_end, 0);    //x86
	
	if (errno == ERANGE) {
		printf("Warning: your number bigger then long\n");
		return -2;
	}

	if (num_end != NULL && *num_end) {
		printf("Incorrect enter\n");
		return -3;
	}
	
	if (biggest <= 0) {
		printf("Your number less then zero or not number\n");
		return -4;
	}

	return biggest;
}

