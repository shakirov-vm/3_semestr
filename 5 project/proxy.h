

long get_num(int argc, char** argv);

struct parent {
	int num;
	int pipefd[2]; 
	char* buf;
	long size;
	long capacity;
	long offset;
};

struct child {
	int num;
	int pipefd[2];
	char* buf;
	long capacity;
};