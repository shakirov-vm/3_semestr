#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define MAX_BUF 100
#define FIFO_TRANSMIT "./name_transmit"
#define UNIQ_LENGTH 10

int main() {

	int err = 0;
	
	err = mkfifo(FIFO_TRANSMIT, 0666); // MAKE TRANSMIT FIFO
	if (errno == EEXIST) printf("FIFO_TRANSMIT almost exist\n");

  	int transmit_read = open(FIFO_TRANSMIT, O_RDONLY);

	int readed;

	char FIFO_uniq[UNIQ_LENGTH];

  	readed = read(transmit_read, FIFO_uniq, UNIQ_LENGTH);
  	if (readed != UNIQ_LENGTH) printf("We read from transmit FIFO only %d\n", readed);
	
	err = mkfifo(FIFO_uniq, 0666);
	if (errno == EEXIST) printf("FIFO_uniq [%s] almost exist\n", FIFO_uniq);

	int uniq_read = open(FIFO_uniq, O_RDONLY );
	if (uniq_read == -1) {
		printf("Can't open fifo file for read\n");
		return 7;
	}

	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); //!
	if (buf == NULL) {
		printf("Can't allocate memory for buf");
		return 6;
	}

	while(1) {
		
		readed = read(uniq_read, buf, MAX_BUF);

		if (readed == 0) break;

		printf("%s\t %d \n", buf, readed);

		if (readed != MAX_BUF) {
			readed = read(uniq_read, buf, MAX_BUF);
			if (readed == 0) {			
				printf("We can't read file, %d readed\n", readed);
				close(uniq_read);

				return 4;
			}
		}
	}

	close(uniq_read);

	printf("\nExit from reader\n");

	return 0;
}