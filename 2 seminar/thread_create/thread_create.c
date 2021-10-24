#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

int inc = 0;

long read_num_args (char* argv);

void* increment () {
	for (size_t i = 0; i < 10000; i++) inc++;
}

int main(size_t argc, char** argv) {

	if (argc != 2) {
		printf("Need 2 args, you enter %ld\n", argc);
		return 1;
	}

	long biggest = read_num_args(argv[1]);

	int status_addr = 0;
	pthread_t* threads = (pthread_t*) calloc (biggest, sizeof(pthread_t*));

	for (int i = 0; i < biggest; i++) {
		pthread_create(threads + i, NULL, increment, NULL);
	} 

	for (int i = 0; i < biggest; i++) {
		pthread_join(threads[i], (void**)&status_addr);
	}

	printf("Number - %d\n", inc);


	free(threads);
	return 0;
}

long read_num_args (char* argv) { //maybe in errno??

	char* num_end;
	long num = strtol(argv, &num_end, 0);    //x86
	if (errno == ERANGE) {
	printf("Warning: your number bigger then long\n");
	exit(3);
	}

	if (num_end != NULL && *num_end) {
	printf("Incorrect enter\n");
	exit(4);
	}
	
	return num;
}