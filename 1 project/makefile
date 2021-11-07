compile_wr:
	gcc writer.c -o writer
compile_rd:
	gcc reader.c -o reader
run_wr:
	./writer number.txt
run_rd:
	./reader
run_wr_1:
	./writer one
run_wr_2:
	./writer two
run_wr_3:
	./writer three
run_wr_easy:
	./writer easy



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

	char FIFO_uniq[UNIQ_LENGTH] = "uniqXXXXXX";
	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); 
	int readed, writed, err;


  	mkstemp(FIFO_uniq);
	remove(FIFO_uniq);
	mkfifo(FIFO_TRANSMIT, 0666); 
	int transmit_write = open(FIFO_TRANSMIT, O_WRONLY);
	writed = write(transmit_write, FIFO_uniq, UNIQ_LENGTH);
	close(transmit_write);

//exit(100); //BLOCK HERE
	int uniq_read = open(FIFO_uniq, O_RDONLY | O_NONBLOCK);
	int val = fcntl (uniq_read, F_SETFL, O_RDONLY);
	if (val == -1) printf("ERROR IN FCNTL\n");
	if (val == 0) printf("NO ERROR IN FCNTL\n");

	while(1) {

		readed = read(uniq_read, buf, MAX_BUF);
printf("After  read - %d\n", readed);
		if (readed == 0) break;

		printf("{%s}\n", buf);

		if (readed != MAX_BUF) {
			readed = read(uniq_read, buf, MAX_BUF);
			if (readed == 0) return 4;
		}
	}

	return 0;
}

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

	char FIFO_uniq[UNIQ_LENGTH];
	char* buf = (char*) calloc (MAX_BUF, sizeof(char));
	int writed;

	mkfifo(FIFO_TRANSMIT, 0666);
  	int transmit_read = open(FIFO_TRANSMIT, O_RDONLY);
  	int readed = read(transmit_read, FIFO_uniq, UNIQ_LENGTH);
  	if (readed != UNIQ_LENGTH) {
  		printf("We read from TRANSMIT %d\n", readed);
  		return 15;
  	}
  	close(transmit_read);
exit(100); //INFINITY CIRCLE THERE
	mkfifo(FIFO_uniq, 0666);

	int data_read = open(argv[1], O_RDONLY);
	int uniq_write = open(FIFO_uniq, O_WRONLY);
	while(1) {

		readed = read(data_read, buf, MAX_BUF); 
	
		if (readed != MAX_BUF) {	
			readed = read(data_read, buf, MAX_BUF); 
			if (readed != 0) return 4;
		}

		printf("[%s]\n", buf);
		writed = write(uniq_write, buf, readed); 

		if (writed != readed) return 5;	
		if (readed == 0) break;
	}

	return 0;
}