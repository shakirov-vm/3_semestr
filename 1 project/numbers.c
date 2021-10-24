#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	/*
	char FIFO_uniq[10] = "uniqXXXXXX";
	int err = mkstemp(FIFO_uniq);
	if (err == -1) printf("Error in mkstemp - [%s]\n", FIFO_uniq);
	
	//execvp(argv[1], argv + 1);

	printf("\n\n");

	err = remove(FIFO_uniq);
	if (err ==  0) printf("We remove [%s]\n", FIFO_uniq);
	if (err == -1) printf("Error in remove\n");
	execvp(argv[1], argv + 1);

	//err = mkfifo(FIFO_uniq, 0666);
	//if (err == -1) printf("Error in MKFIFO\n");

	*/
	FILE* potok = fopen("three", "w");
	
	for(size_t i = 0; i < 100000000; i++) {
		fprintf(potok, "%d", 3);
		if (i % 20 == 0) fprintf(potok, "\n");
	}
	
	fclose(potok);
	return 0;
}
