
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
#define BUFSIZE 5

void child_dead(int sig);
void child_one(int sig);
void child_two(int sig);
void parent_one(int sig);
void parent_two(int sig);

int bit = 0;

int main(int argc, char** argv) {

	int err = 0;

	if (argc != 2) {
		printf("Invalid number of args\n");
		exit(1);
	}

	sigset_t init_mask;
	sigfillset(&init_mask);
	sigdelset(&init_mask, SIGCHLD);
	err = sigprocmask(SIG_SETMASK, &init_mask, NULL);
	if (err == -1) {
		perror("init sigprocmask ");
		exit(1);
	}

	sigset_t set_child_dead;
	sigfillset(&set_child_dead);

	struct sigaction set_sig_child_handle;
	set_sig_child_handle.sa_handler = child_dead;
	set_sig_child_handle.sa_mask = set_child_dead;
	set_sig_child_handle.sa_flags = SA_NOCLDWAIT;
	err = sigaction(SIGCHLD, &set_sig_child_handle, NULL);
	if (err == -1) {
		perror("sigaction child_dead ");
		exit(1);
	}
	
//	sigaction//
// Отсюда и до строки за сигнал от родителя ребёнку
	int pid = fork();
//Если USR1 придёт в этот момент, то зависнем на sigsuspend
//	sigprocmask

	if (pid == -1) {
		perror("fork ");
		exit(1);
	}

	if (pid == 0) { //child
// Отсюда до сравнения с getppid идёт борьба за маску - за установку prctl
		err = prctl(PR_SET_PDEATHSIG, SIGKILL);
		if (err == -1) {
			perror("prctl ");
			exit(1);
		}
	        int ppid = getppid();
	        if (ppid == 1) {
	        	printf("parent is dead\n");
	        	exit(1);
	        }

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
		err = sigaction(SIGUSR1, &set_sig_usr1_handle, NULL);
		if (err == -1) {
			perror("sigaction USR1 in child ");
			exit(1);
		}

		struct sigaction set_sig_usr2_handle;
		set_sig_usr2_handle.sa_handler = child_two;
		set_sig_usr2_handle.sa_mask = full_set;
		set_sig_usr2_handle.sa_flags = 0;
		err = sigaction(SIGUSR2, &set_sig_usr2_handle, NULL);
		if (err == -1) {
			perror("sigaction USR2 in child ");
			exit(1);
		}

		sigset_t parent_ready = {};
	        sigfillset(&parent_ready);
	        sigdelset(&parent_ready, SIGUSR1);
	        sigsuspend(&parent_ready);
	//Вот до сюда от fork - race condition - за то, чтобы сигнал пришёл в маску пришедших сигналов ребёнка или блок
	        char buf[BUFSIZE] = {};
	        char symbol = 0;
	        int readed = 0;
	        int bit_write = 0;

	        while(1) {

	        	readed = read(fd, buf, BUFSIZE);

	        	if (readed == -1) {
	        		perror("read ");
	        		exit(1);
	        	}

	        	for(int buf_pos = 0; buf_pos < readed; buf_pos++) {

	        		symbol = buf[buf_pos];

	        		for(int i = 0; i < 8; i++) {
	        			bit_write = symbol % 2;
	        			symbol /= 2;
// Вот тут гонка за маску родителя
	        			if (bit_write == 0) {
	        				err = kill(ppid, SIGUSR1);
	        				if (err == -1) {
	        					perror("kill 1 from child ");
	        					exit(1);
	        				}
	        			}

	        			if (bit_write == 1) {
	        				err = kill(ppid, SIGUSR2);
	        				if (err == -1) {
	        					perror("kill 2 from child ");
	        					exit(1);
	        				}
	        			}

	        			sigset_t set_bit;
					sigfillset(&set_bit);
					sigdelset(&set_bit, SIGUSR1);
					//Гонка в том, что один посылает сигнал, а у второго он либо замаскирован, либо он принимает - в цикле
					sigsuspend(&set_bit); //тут получаем возможность принять SIGUSR1, или взять его, если он пришёл раньше
        			}
	        	}

	        	if (readed == 0) break;
	        }

	} else { //parent

		sigset_t full_set;
		sigfillset(&full_set);
		sigdelset(&full_set, SIGCHLD); // Здесь если прилетает SIGCHLD - выходим

		struct sigaction set_sig_usr1_handle;
		set_sig_usr1_handle.sa_handler = parent_one;
		set_sig_usr1_handle.sa_mask = full_set;
		set_sig_usr1_handle.sa_flags = 0;
		err = sigaction(SIGUSR1, &set_sig_usr1_handle, NULL);
		if (err == -1) {
			perror("sigaction USR1 in parent ");
			exit(1);
		}

		struct sigaction set_sig_usr2_handle;
		set_sig_usr2_handle.sa_handler = parent_two;
		set_sig_usr2_handle.sa_mask = full_set;
		set_sig_usr2_handle.sa_flags = 0;
		err = sigaction(SIGUSR2, &set_sig_usr2_handle, NULL);
		if (err == -1) {
			perror("sigaction USR2 in parent ");
			exit(1);
		}

		err = kill(pid, SIGUSR1);
		if (err == -1) {
			perror("kill ready from parent ");
			exit(1);
		}

		char symbol = 0;

		while(1) {

			symbol = 0;

			for(int i = 0; i < 8; i++) {
				sigset_t get_bit;
				sigfillset(&get_bit);
				sigdelset(&get_bit, SIGUSR1);
				sigdelset(&get_bit, SIGUSR2);
				sigdelset(&get_bit, SIGCHLD);
// такая же ситуация - борьба за маску ребёнка - от sigsuspend до sigsuspend
				sigsuspend(&get_bit);
				
				symbol = symbol | (bit << i);

				err = kill(pid, SIGUSR1);
				if (err == -1) {
					perror("kill from parent ");
					exit(1);
				}
			}

			printf("%c", symbol);
			if (symbol == 0) break;
		}
	}
}

void child_dead(int sig) {
	printf("\nChild is end\n");
	exit(0);
}

void child_one(int sig) {}
void child_two(int sig) {}
//Вот здесь 2 обработчика борятся за глобальную переменную
void parent_one(int sig) {
	bit = 0;
	//printf("Come 0\n");
}
void parent_two(int sig) {
	bit = 1;
	//printf("Come 1\n");
}