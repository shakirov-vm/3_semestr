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

int main(int argc, char* argv[]) {

	int readed, writed, err;
	char* buf = (char*) calloc (MAX_BUF, sizeof(char));
	if (buf == NULL) {
		printf("Can't allocate memory for buf");
		return 6;
	}
	

	char FIFO_uniq[UNIQ_LENGTH];

	err = mkfifo(FIFO_TRANSMIT, 0666); // MAKE TRANSMIT FIFO
	if (errno == EEXIST) printf("FIFO_TRANSMIT almost exist\n");

  	int transmit_read = open(FIFO_TRANSMIT, O_RDONLY);
	if (transmit_read == -1) {
		printf("Can't open transmit_fifo file for read\n");
		return 7;
	}

  	readed = read(transmit_read, FIFO_uniq, UNIQ_LENGTH);
  	if (readed != UNIQ_LENGTH) {
  		printf("We read from transmit FIFO only %d\n", readed);
  		return 9;
	}
  	close(transmit_read);


	err = mkfifo(FIFO_uniq, 0666);
	if (errno == EEXIST) printf("FIFO_uniq [%s] almost exist\n", FIFO_uniq);
printf("BEFORE OPEN\n");
	int data_read = open(argv[1], O_RDONLY);
	int uniq_write = open(FIFO_uniq, O_WRONLY);
printf("AFTER OPEN\n");
	if (data_read == -1) {
		printf("Can't open text file for read\n");
		return 7;
	}
	if (uniq_write == -1) {
		printf("Can't open fifo file for write\n");
		return 7;
	}

	while(1) {

		readed = read(data_read, buf, MAX_BUF); 
	
		if (readed != MAX_BUF) {
			
			readed = read(data_read, buf, MAX_BUF); 
			if (readed != 0) {	
				printf("We can't read file, %d readed\n", readed);	
				free(buf);	
				close(data_read); 
				close(uniq_write);
		
				return 4;
			}
		}

		writed = write(uniq_write, buf, readed); 
printf("WRITED - %d\n", writed);

		printf("%s", buf);

		if (writed != readed) {
			printf("We can't write correctly, %d writed\n", writed);
			free(buf);
			close(data_read); 
			close(uniq_write);
		
			return 5;
		}
		
		if (readed == 0)
			break;
	}

	close(data_read);
	close(uniq_write);

	printf("Exit from writer\n");

	return 0;
}