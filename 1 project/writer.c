
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define MAX_BUF 40
#define FIFO_NAME "./tmp001"

int main(int argc, char* argv[]) {

	int erfifo = mkfifo(FIFO_NAME, 0777); //!
	if (errno == EEXIST) printf("FIFO almost exist\n");
	if (erfifo == -1) printf("Error in FIFO\n"); 

	int data_stream_read = open(argv[1], O_RDONLY); //!
	int fifo_stream_write = open(FIFO_NAME, O_WRONLY); //!

	if (data_stream_read == -1) printf("Can't open text file for read\n");
	if (fifo_stream_write == -1) printf("Can't open fifo file for write\n");

	char* buf = (char*) calloc (MAX_BUF, sizeof(char)); //!

	int readed, writed;

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
		printf("Write %d byte\n", writed);
		
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