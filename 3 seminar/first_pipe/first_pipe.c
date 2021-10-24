
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>


#define MAX_BUF 2
/*
int main (size_t argc, char** argv) {
	//argv[2] - name of file

	int pipefd[2];
	int error = pipe(pipefd);

	FILE* potok = fopen(argv[1], "r");

	char* buffer = (char*) calloc(MAX_BUF, sizeof(char));
	char buf;
	int handle = -10;

	pid_t pid = 1;
	pid = fork();

	if (pid == 0) {//child
		int readed = fread(buffer, sizeof(char), MAX_BUF, potok);
		printf("read from [%s] in child - %d\n", argv[1], readed);
		while (write(pipefd[1], buffer, 1) > 0) {
			printf("Write in pipe\n");
		}
	}
}*/

int main (size_t argc, char** argv) {
	//argv[2] - name of file
	int pipefd[2];
	int error = pipe(pipefd);

	pid_t pid = 0;
	pid = fork();

	if (pid == 0) { //child
		close(pipefd[0]); //close read

		char* buf = (char*) calloc (MAX_BUF, sizeof (char));
		int readed = -1;
		int writed = -1;

		int potok = open(argv[1], O_RDONLY);
		if (potok == -1) printf("Problem with open\n");

		do {
			readed = read(potok, buf, MAX_BUF);
			if (readed > 0) {
				writed = write(pipefd[1], buf, MAX_BUF);
				printf("In child we read [%s], readed = %d\n", buf, readed);
			}
		} while (writed > 0 && readed > 0);
	}

	if (pid != 0) { //parent
		close(pipefd[1]); //close write
		
		char* buf = (char*) calloc (MAX_BUF, sizeof (char));
		int readed = 1;

		do {
			readed = read(pipefd[0], buf, MAX_BUF);
			if (readed > 0) {
				printf("In parent - [%s], readed = %d\n", buf, readed);
			}
		} while(readed > 0);
	}
	return 0;
}



/*
	while (1) {
		if (pid == 0) {// daughter

			while (handle != 0) {
				fread(buffer, 1, 1, potok); //sizeof pipe
				printf("in child read - %d\n", handle);
				handle = write(pipefd[1], buffer, 1);
				printf("in child write - %d\n", handle);
			}
			
		}

		else {// parent

			while(handle != 0) {
				close(pipefd[1]);
				handle = read(pipefd[0], buffer, 1);
				printf("%s", buffer);
			}
		}
	}
*/