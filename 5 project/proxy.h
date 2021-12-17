

long get_num(int argc, char** argv);

struct parent {
	int num;
	pid_t pid;
	int pipefd[2]; // Dynamic?
	char* buf;
	long size;
	long capacity; // capacity
	long offset;
};

struct child {
	int num;
	pid_t ppid;
	int pipefd[2];
	char* buf;
	long capacity;
};