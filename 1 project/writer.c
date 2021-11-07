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

	//char FIFO_uniq[UNIQ_LENGTH] = "uniqXXXXXX";
	int err = 0;
	char FIFO_uniq[UNIQ_LENGTH];
	char* buf = (char*) calloc (MAX_BUF, sizeof(char));
	if (buf == NULL) {
		perror("Buf alloc ");
	}
	int writed;

	err = mkfifo(FIFO_TRANSMIT, 0666);
	if (err == -1) {
		perror("mk transmit fifo ");
	}
// 1
  	int transmit_read = open(FIFO_TRANSMIT, O_RDONLY);
  	if (transmit_read == -1) {
  		perror("open transmit for read ");
  	}

	mkfifo(FIFO_uniq, 0666);
	if (err == -1) {
		perror("mk transmit fifo ");
	}
//exit(100);
	int data_read = open(argv[1], O_RDONLY);
	if (data_read == -1) {
		perror("open data for read ");
	}
// 2
	int readed = read(transmit_read, FIFO_uniq, UNIQ_LENGTH);
  	if (readed != UNIQ_LENGTH) {
  		if (readed == -1) perror("read from transmit");
  		printf("We read from TRANSMIT %d\n", readed);
  		return 15;
  	}
  	//exit(100);
// 3
//sleep(10);
	err = mkfifo(FIFO_uniq, 0666);
	if (err == -1) {
		perror("mk uniq fifo ");
	}

	int uniq_write = open(FIFO_uniq, O_WRONLY);// | O_NONBLOCK);
	if (uniq_write == -1) {
		printf("Uniq is [%s]\n", FIFO_uniq);
		perror("open uniq for write ");
	}

	int val = fcntl(uniq_write, F_SETFL, O_WRONLY);
	if (val == -1) {
		printf("Fcntl is bad\n");
		perror("fcntl failed ");
	}

	while(1) {

		readed = read(data_read, buf, MAX_BUF); 
		if (readed == -1) {
			perror("read from data ");
		}

		if (readed != MAX_BUF) {	
			readed = read(data_read, buf, MAX_BUF); 
			if (readed != 0) return 4;
		}

		writed = write(uniq_write, buf, readed); 
		if (writed == -1) {
			perror("write in uniq ");
		}
		printf("%s\n", buf);

		if (writed != readed) {
			printf("readed - %d, writed - %d\n", readed, writed);
			return 5;
		}	
		if (readed == 0) break;
	}

	return 0;
}