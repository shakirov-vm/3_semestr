
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#define filename argv[1] 
#define BUFSIZE 256

void child_dead(int sig);
void child_one(int sig);
void child_two(int sig);
void parent_one(int sig);
void parent_two(int sig);

int bit = 0;

int main(int argc, char** argv) {

	int err = 0;

	if (argc != 2) {
		printf("Invalid number of ar(gs\n");
				exit(1);
	}

	sigset_t init_mask;
	sigfillset(&init_mask);
	sigdelset(&init_mask, SIGCHLD);
	sigdelset(&init_mask, SIGINT);
	sigprocmask(SIG_SETMASK, &init_mask, NULL);
//sleep(5);
	sigset_t set_start;
	sigfillset(&set_start);

	struct sigaction set_sig_child_handle;
	set_sig_child_handle.sa_handler = child_dead;
	set_sig_child_handle.sa_mask = set_start;
	set_sig_child_handle.sa_flags = SA_NOCLDWAIT;
	err = sigaction(SIGCHLD, &set_sig_child_handle, NULL);
	if (err == -1) {
		perror("sigaction child_dead ");
		exit(1);
	}


	int pid = fork();

	if (pid == 0) { //child

		prctl (PR_SET_PDEATHSIG, SIGKILL);
		
		int fd = open(filename, O_RDONLY);
		if (fd == -1) {
			perror("open ");
			exit(1);
		}

		sigset_t full_set;
		sigfillset(&full_set);

		struct sigaction set_sig_usr1_handle;
		set_sig_usr1_handle.sa_handler = child_one;
		set_sig_usr1_handle.sa_mask = full_set;
		set_sig_usr1_handle.sa_flags = 0;
		sigaction(SIGUSR1, &set_sig_usr1_handle, NULL);

		struct sigaction set_sig_usr2_handle;
		set_sig_usr2_handle.sa_handler = child_two;
		set_sig_usr2_handle.sa_mask = full_set;
		set_sig_usr2_handle.sa_flags = 0;
		sigaction(SIGUSR2, &set_sig_usr2_handle, NULL);

		sigset_t parent_ready = {};
        sigfillset (&parent_ready);
        sigdelset (&parent_ready, SIGUSR1);
        sigsuspend (&parent_ready);
        sigaddset (&parent_ready, SIGUSR1);

        char buf[BUFSIZE] = {};
        char symbol = 0;
        int readed = 0;
        int ppid = getppid();
        int bit_write = 0;

        while(1) {
        	readed = read(fd, buf, BUFSIZE);
        	printf("BUF: [%s]\n\n", buf);
        	if (readed == -1) {
        		//How we handle there?
        		printf("What we do there?\n");
        	}

        	for(int buf_pos = 0; buf_pos < readed; buf_pos++) {

        		symbol = buf[buf_pos];
				//printf("child - [%d]\n", symbol);

        		for(int i = 0; i < 8; i++) {
        			bit_write = symbol % 2;
        			symbol /= 2;

        			if (bit_write == 0) {
        				kill(ppid, SIGUSR1);
        			}

        			if (bit_write == 1) {
        				kill(ppid, SIGUSR2);
        			}

        			sigset_t set_bit;
					sigfillset (&set_bit);
					sigdelset (&set_bit, SIGUSR1);
					sigsuspend (&set_bit);
        		}
        	}

        	if (readed == 0) break;
        }

	} else { //parent

		sigset_t full_set;
		sigfillset(&full_set);
		sigdelset(&full_set, SIGCHLD); // Здесь если прилетает SIGCHLD - выходим
		//sigdelset(&full_set, SIGINT);

		struct sigaction set_sig_usr1_handle;
		set_sig_usr1_handle.sa_handler = parent_one;
		set_sig_usr1_handle.sa_mask = full_set;
		set_sig_usr1_handle.sa_flags = 0;
		sigaction(SIGUSR1, &set_sig_usr1_handle, NULL);

		struct sigaction set_sig_usr2_handle;
		set_sig_usr2_handle.sa_handler = parent_two;
		set_sig_usr2_handle.sa_mask = full_set;
		set_sig_usr2_handle.sa_flags = 0;
		sigaction(SIGUSR2, &set_sig_usr2_handle, NULL);

		kill (pid, SIGUSR1);

		char symbol = 0;

		while(1) {

			symbol = 0;

			for(int i = 0; i < 8; i++) {
				sigset_t get_bit;
				sigfillset (&get_bit);
				sigdelset (&get_bit, SIGUSR1);
				sigdelset (&get_bit, SIGUSR2);
				sigdelset (&get_bit, SIGCHLD);

				sigsuspend (&get_bit);
				
				symbol = symbol | (bit << i);

				kill(pid, SIGUSR1);
			}

			printf("parent - [%d]\n", symbol);
			if (symbol == 0) break;
		}
	}
}

void child_dead(int sig) {
	printf("Child is dead\n");
	exit(1);
}

void child_one(int sig) {}
void child_two(int sig) {}
void parent_one(int sig) {
	bit = 0;
	//printf("Come 0\n");
}
void parent_two(int sig) {
	bit = 1;
	//printf("Come 1\n");
}