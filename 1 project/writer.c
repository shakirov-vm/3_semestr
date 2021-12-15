#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define MAX_BUF 2000
#define FIFO_TRANSMIT "./name_transmit"
#define UNIQ_LENGTH 10

int check_the_same(int one_fd, int two_fd);

int main(int argc, char* argv[]) {

	int err = 0;
	int writed;
	char FIFO_uniq[UNIQ_LENGTH];
	char* buf = (char*) calloc (MAX_BUF, sizeof(char));
	if (buf == NULL) {
		perror("Buf alloc ");
		return 1;
	}

	err = mkfifo(FIFO_TRANSMIT, 0666);
	if (err == -1 && errno != EEXIST) {
		perror("mk transmit fifo ");
		return 2;
	}

  	int transmit_read = open(FIFO_TRANSMIT, O_RDONLY);
  	if (transmit_read == -1) {
  		perror("open transmit for read ");
  		return  3;
  	}

	int data_read = open(argv[1], O_RDONLY);
	if (data_read == -1) {
		perror("open data for read ");
		return 3;
	}
// вот тут критическая секция - между writer'ами идёт борьба за уникальное имя из FIFO
//От сюда
	int readed = read(transmit_read, FIFO_uniq, UNIQ_LENGTH);
  	if (readed != UNIQ_LENGTH) {
  		if (readed == -1) perror("read from transmit");
  		printf("We read from TRANSMIT %d\n", readed);
  		return 4;
  	}

  	printf("Get uniq fifo name - [%s]\n", FIFO_uniq);

	err = mkfifo(FIFO_uniq, 0666);
	if (err == -1 && errno != EEXIST) {
		perror("mk uniq fifo ");
		return 2;
	}
//Отсюда и до 84 строки идёт борьба между reader'ом и writer'ом за изменение информации в ядре об открытии FIFO
//До сюда
	int uniq_write = open(FIFO_uniq, O_WRONLY | O_NONBLOCK);
	if (uniq_write == -1) {
		printf("Uniq is [%s]\n", FIFO_uniq);
		perror("open uniq for write ");
		return 3;
	}

	int val = fcntl(uniq_write, F_SETFL, O_WRONLY);
	if (val == -1) {
		printf("Fcntl is bad\n");
		perror("fcntl failed ");
		return 5;
	}

	printf("Processes connect\n");

	while(1) {

		readed = read(data_read, buf, MAX_BUF); 
		if (readed == -1) {
			perror("read from data ");
			return 4;
		}

		writed = write(uniq_write, buf, readed); 
		if (writed == -1) {
			perror("write in uniq ");
			return 4;
		}

		//printf("%s", buf);

		if (writed != readed) {
			printf("readed - %d, writed - %d\n", readed, writed);
			return 4;
		}	
		if (readed == 0) break;
	}
	printf("\n");
	printf("Write end\n");
	return 0;
}

int check_the_same(int one_fd, int two_fd) {

	struct stat one_stat;
	struct stat two_stat;

	fstat(one_fd, &one_stat);
	fstat(two_fd, &two_stat);

	if (one_stat.st_ino == two_stat.st_ino && one_stat.st_dev == two_stat.st_dev) 
		return 1;
	return 0;
}