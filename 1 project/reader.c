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

	int readed, writed, err;
	char* buf = (char*) calloc (MAX_BUF, sizeof(char));
	if (buf == NULL) {
		printf("Can't allocate memory for buf");
		return 6;
	}


	char FIFO_uniq[UNIQ_LENGTH] = "uniqXXXXXX";
	
	err = mkstemp(FIFO_uniq);
	if (err == -1) {
		printf("Can't create uniq name of file\n");
		return 8;
	}

	err = remove(FIFO_uniq);
	if (err == -1) {
		printf("Can't remove uniq name of file\n");
		return 8;
	}

	err = mkfifo(FIFO_TRANSMIT, 0666); // MAKE TRANSMIT FIFO
	if (errno == EEXIST) printf("FIFO_TRANSMIT almost exist\n");
	
	int transmit_write = open(FIFO_TRANSMIT, O_WRONLY);
	if (transmit_write == -1) {
		printf("Can't open transmit_fifo file for write\n");
		return 7;
	}

	writed = write(transmit_write, FIFO_uniq, UNIQ_LENGTH);
	if (writed != UNIQ_LENGTH) printf("We write in transmit FIFO only %d\n", writed);

	close(transmit_write);


	
	err = mkfifo(FIFO_uniq, 0666);
	if (errno == EEXIST) printf("FIFO_uniq [%s] almost exist\n", FIFO_uniq);

	int uniq_read = open(FIFO_uniq, O_RDONLY);// | O_NONBLOCK);
	if (uniq_read == -1) {
		printf("Can't open fifo file for read\n");
		return 7;
	}

	int val = fcntl(uniq_read, F_GETFL, 0);
	printf("First val - %d\n", val);
	int flags = O_NONBLOCK;
	val &= ~flags;
	fcntl(uniq_read, F_SETFL, val);
	val = fcntl(uniq_read, F_GETFL, 0);
	printf("Secon val - %d\n", val);
	
//Тут уже не должно быть нонблока
	while(1) {
		
		readed = read(uniq_read, buf, MAX_BUF);
printf("READED - %d\n", readed);
		if (readed == 0) break;

		printf("%s", buf);

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