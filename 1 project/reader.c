
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#define MAX_BUF 40
#define FIFO_NAME "./tmp001"

int main() {

	int erfifo = mkfifo(FIFO_NAME, 0777); //!
	if (erfifo == -1) printf("Error in FIFO\n");

	int fifo_stream = open(FIFO_NAME, O_RDONLY ); //!
	if (fifo_stream == -1) printf("Can't open fifo file for read\n");

	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); //!

	int readed;

	while(1) {
		//readed = 0; 
		
		readed = read(fifo_stream, buf, MAX_BUF); // feof need
		//printf("Read %d byte\n", readed);

		if (readed == 0) break;

		printf("%s", buf);

		if (readed != MAX_BUF) {
			readed = read(fifo_stream, buf, MAX_BUF);
			if (readed == 0) {			
				printf("We can't read file, %d readed\n", readed);
				close(fifo_stream);

				return 4;
			}
		}
	}

	close(fifo_stream);

	printf("\nExit from reader\n");

	return 0;
}