
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define MAX_BUF 40
#define FIFO_TRANSMIT "./name_transmit"
#define UNIQ_LENGTH 10

int main() {

	int err = 0;

	
	err = mkfifo(FIFO_TRANSMIT, 0666); // MAKE TRANSMIT FIFO
	if (errno == EEXIST) printf("FIFO_TRANSMIT almost exist\n");

  	int transmit_read = open(FIFO_TRANSMIT, O_RDONLY);
//////


	int readed;

	char FIFO_uniq[UNIQ_LENGTH];

  	readed = read(transmit_read, FIFO_uniq, UNIQ_LENGTH);
  	if (readed != UNIQ_LENGTH) printf("We read from transmit FIFO only %d\n", readed);

printf("From writer: [%s], write: %d, L_tmpnam: %d\n", FIFO_uniq, readed, UNIQ_LENGTH);

	
	err = mkfifo(FIFO_uniq, 0666);
	if (errno == EEXIST) printf("FIFO_uniq [%s] almost exist\n", FIFO_uniq);

	int fifo_stream = open(FIFO_uniq, O_RDONLY ); //!
	if (fifo_stream == -1) printf("Can't open fifo file for read\n");

	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); //!


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