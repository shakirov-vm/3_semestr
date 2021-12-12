
#include "proxy.h"

struct parent {
	int num;
	pid_t pid;
	int pipefd[2]; // Dynamic?
	char* buf;	
};

struct child {
	int num;
	pid_t ppid;
	int pipefd[2];
	char* buf;
};
// argv[1] - num of child, argv[2] - file name
int main(int argc, char** argv) {

	long num = get_num(argc, argv);

	struct parent* childs = (struct parent*) calloc (num, sizeof(struct parent));

	int new_pid = -1;
	int my_num = -1; //parent have -1
	int tmp_pipefd[2] = {0};
	pid_t true_ppid = getpid();

	for(int i = 0; i < num; i++) {

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

			struct child* i_am = (struct child*) calloc (1, sizeof(struct child));
			i_am.num = i;
			i_am.ppid = true_ppid;
			i_am.buf_size = (long) pow(3, N - i + 4);
			i_am.buf = (char*) calloc (i_am[i].buf_size, sizeof(char)); 
			i_am.pipefd[0] = tmp_pipefd_from_parent[0];
			i_am.pipefd[1] = tmp_pipefd_to_parent[1];

			free(childs); //Must we do this?

			break;
		}

		// parent

		close(tmp_pipefd_to_parent[1]); // close write
		close(tmp_pipefd_from_parent[0]); // close read

		childs[i].num = i;
		childs[i].pid = new_pid;
		childs[i].buf_size = (long) pow(3, N - i + 4);
		childs[i].buf = (char*) calloc (childs[i].buf_size, sizeof(char));
		childs[i].pipefd[0] = tmp_pipefd_to_parent[0];
		childs[i].pipefd[1] = tmp_pipefd_from_parent[1];
// Is it normal?
		fcntl(childs[i].pipefd[0], F_SETFL, O_RDONLY | O_NONBLOCK);
		fcntl(childs[i].pipefd[1], F_SETFL, O_WRONLY | O_NONBLOCK);
	}

	// child

	if (new_pid == 0) {
	
		if (my_num == 0) {

			i_am.pipefd[0] = open(argv[2], O_RDONLY); // Maybe need be more carefull
			if (pipefd[0] == -1) {
				perror("open file ");
				exit(1);
			}
		}

		if (my_num == num - 1) {

			i_am.pipefd[1] = stdout;
		}

		while(1) {
			
			int readed = read(i_am.pipefd[0], i_am.buf, i_am.buf_size);
			if (readed == -1) {
				perror("read in child ");
				exit(1);
			}

			int writed = write(i_am.pipefd[1], i_am.buf, readed);
			if (writed != readed) {
				printf("We write less than read\n");
			}

			if (readed == 0) break;
		}

	}

	else { // parent


	}

	if (my_num == -1) free(childs);
	if (my_num != -1) free(i_am);
}

long get_num(int argc, char** argv) {

	if (argc != 2) {
		printf("Need 2 args, you enter %ld\n", argc);
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