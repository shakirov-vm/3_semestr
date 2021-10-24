#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

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
	printf("Before circle\n");
	for(size_t i = 0; i < biggest; i++) {
		if (pid > 0) {
			pid = fork();
		}

		if (pid == 0) {
			printf("Child %ld, pid - %d, ppid - %d\n", i, getpid(), getppid());
			break;
		}
		if (pid < 0) {
			printf("Error with fork\n");
			return 5;
		}
	}
	return 0;
}

/*
int argv_handle (char* char_num) {
	char* num_end;

	long num = strtol(char_num, &num_end, 0);    //x86
	if (errno == ERANGE) {
		printf("Warning: your number bigger then long\n");
		exit(3);
	}

	printf("There\n");
	if (num_end != NULL && *num_end) {
		printf("Incorrect enter\n");
		exit(3);
	}

	return num;
}*/