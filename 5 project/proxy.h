

long get_num(int argc, char** argv);

struct parent {
	int num;
	pid_t pid;
	int pipefd[2]; // Dynamic?
	char* buf;
	long bif_size;

};

struct child {
	pid_t ppid;
	int pipefd[2];
	char* buf;
	long bif_size;
};