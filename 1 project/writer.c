
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

int main(int argc, char* argv[]) {

	int err = 0;

	char FIFO_uniq[UNIQ_LENGTH] = "uniqXXXXXX";
	
	err = mkstemp(FIFO_uniq);
	if (err == -1) printf("Can't create uniq name of file\n");

	err = remove(FIFO_uniq);
	if (err == -1) printf("Can't remove uniq name of file\n");

	err = mkfifo(FIFO_TRANSMIT, 0666); // MAKE TRANSMIT FIFO
	if (errno == EEXIST) printf("FIFO_TRANSMIT almost exist\n");

	err = mkfifo(FIFO_uniq, 0666);
	if (errno == EEXIST) printf("FIFO_uniq [%s] almost exist\n", FIFO_uniq);
	
	int readed, writed;

	int transmit_write = open(FIFO_TRANSMIT, O_WRONLY); 
 ////////
	
	writed = write(transmit_write, FIFO_uniq, UNIQ_LENGTH);
	if (writed != UNIQ_LENGTH) printf("We write in transmit FIFO only %d\n", writed);



	int data_stream_read = open(argv[1], O_RDONLY); //!
	int fifo_stream_write = open(FIFO_uniq, O_WRONLY); //!


	if (data_stream_read == -1) printf("Can't open text file for read\n");
	if (fifo_stream_write == -1) printf("Can't open fifo file for write\n");

	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); //!


	while(1) {
		readed = 0; //!
		writed = 0;
		
		readed = read(data_stream_read, buf, MAX_BUF); 
	
		if (readed != MAX_BUF) {
			
			readed = read(data_stream_read, buf, MAX_BUF); 
			if (readed != 0) {	
				printf("We can't read file, %d readed\n", readed);		
				close(data_stream_read); // We always can close correct?
				close(fifo_stream_write);
		
				return 4;
			}
		}
	
		writed = write(fifo_stream_write, buf, readed); 
		//printf("Write %d byte\n", writed);
		printf("%s", buf);

		if (writed != readed) {
			printf("We can't write correctly, %d writed\n", writed);
			close(data_stream_read); // We always can close correct?
			close(fifo_stream_write);
		
			return 5;
		}
		
		if (readed == 0)
			break;
	}

	close(data_stream_read); // We always can close correct?
	close(fifo_stream_write);

	printf("Exit from writer\n");

	return 0;
}