#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define MAX_BUF 10
#define FIFO_TRANSMIT "./name_transmit"
#define UNIQ_LENGTH 10

int main() {

	//char FIFO_uniq[UNIQ_LENGTH];
	char FIFO_uniq[UNIQ_LENGTH] = "uniqXXXXXX";
	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); 
	int readed, err;

  	mkstemp(FIFO_uniq);
	remove(FIFO_uniq);
// 1
	err = mkfifo(FIFO_TRANSMIT, 0666);
	if (err == -1) {
		perror("mk transmit fifo ");
	} 
	int transmit_write = open(FIFO_TRANSMIT, O_WRONLY);
	if (transmit_write == -1) {
		perror("transmit open for write ");
	}

//exit(11);
// 2
	int writed = write(transmit_write, FIFO_uniq, UNIQ_LENGTH);
	if (writed == -1) {
		perror("write in transmit ");
	}
//exit(100);
	err = mkfifo(FIFO_uniq, 0666);
	if (err == -1) {
		perror("mk uniq fifo ");
	}
// 3
	int uniq_read = open(FIFO_uniq, O_RDONLY | O_NONBLOCK);
	if (uniq_read == -1) {
		perror("uniq open for read ");
	}
	printf("We open uniq fifo for read - %d\n", uniq_read);
	//поменяьб нонблок

	int val = fcntl(uniq_read, F_SETFL, O_RDONLY);
	if (val == -1) {
		printf("Fcntl is bad\n");
		perror("fcntl failed ");
	}

//exit(10);
	while(1) {
//sleep(20);
		readed = read(uniq_read, buf, MAX_BUF);
		if (readed == -1) {
			perror("read from uniq ");
		}
printf("We read %d\n", readed);
		if (readed == 0) break;

		printf("%s\n", buf);

		if (readed != MAX_BUF) {
			readed = read(uniq_read, buf, MAX_BUF);
			if (readed == 0) return 4;
		}
	}

	return 0;
}