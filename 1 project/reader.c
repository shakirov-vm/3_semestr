#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <stdio.h>
#include <errno.h>

#define MAX_BUF 2000
#define FIFO_TRANSMIT "./name_transmit"
#define UNIQ_LENGTH 10

int main() {

	char FIFO_uniq[UNIQ_LENGTH] = "uniqXXXXXX";
	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); 
	int readed, err;

  	mkstemp(FIFO_uniq);
	remove(FIFO_uniq);

	err = mkfifo(FIFO_TRANSMIT, 0666);
	if (err == -1 && errno != EEXIST) {
		perror("mk transmit fifo ");
		return 2;
	} 

	int transmit_write = open(FIFO_TRANSMIT, O_WRONLY);
	if (transmit_write == -1) {
		perror("transmit open for write ");
		return 3;
	}

	err = mkfifo(FIFO_uniq, 0666);
	if (err == -1 && errno != EEXIST) {
		perror("mk uniq fifo ");
		return 2;
	}
//Отсюда и до 67 строки идёт борьба между reader'ом и writer'ом за изменение информации в ядре об открытии FIFO
	int uniq_read = open(FIFO_uniq, O_RDONLY | O_NONBLOCK);
	if (uniq_read == -1) {
		perror("uniq open for read ");
		return 3;
	}

	int writed = write(transmit_write, FIFO_uniq, UNIQ_LENGTH);
	if (writed == -1) {
		perror("write in transmit ");
		return 4;
	}

	int val = fcntl(uniq_read, F_SETFL, O_RDONLY);
	if (val == -1) {
		printf("Fcntl is bad\n");
		perror("fcntl failed ");
		return 5;
	}

	if (writed != UNIQ_LENGTH) {
		printf("We writed in transmit only %d\n", writed);
		return 4;
	}
	printf("We ready for read\n");
	printf("Wait 3 seconds\n");
	sleep(3);

	while(1) {

		readed = read(uniq_read, buf, MAX_BUF);
		if (readed == -1) {
			perror("read from uniq ");
			return 4;
		}

		if (readed == 0) break;

		printf("%s", buf);

		if (readed != MAX_BUF) continue;
	}
	printf("\n");
	printf("Read end\n");
	return 0;
}