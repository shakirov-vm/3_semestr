#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define megabyte 4096

int main(int argc, char** argv) {
	if (argc != 3) {
	printf("There must be 3 args, you enter %d\n", argc);
	return -1;
	}

	/*struct stat buff;        Bad choice
	stat(input, argv[2]);
	size_t entered = buff.st_size;*/
	
	char buffer[megabyte] = {0}; 
		
	int from = open(argv[1], O_RDONLY);
	if (from == 0) { //??
		printf("FROM file can't be open\n");
		return 2;
	} 
	int to_the = open(argv[2], O_WRONLY);
	if (to_the == 0) {
		printf("TO_THE file can't be open\n");
		return 3;
	} 
	
	size_t readed;
	size_t writed;
	
	while(1) {
		readed = 0;
		writed = 0;
		
		readed = read(from, buffer, megabyte); 
	
		if (readed != megabyte) {
			
			readed = read(from, buffer, megabyte); 
			if (readed != 0) {	

				printf("We can't read file, %lu readed\n", readed);
				
				close(from); // We always can close correct?
				close(to_the);
				return 4;
			}
		}
	
		writed = write(to_the, buffer, readed); 
		
		if (writed != readed) {
			printf("We can't write correctly, %lu writed\n", writed);
			close(from); // We always can close correct?
			close(to_the);
			return 5;
		}
		
		if (readed == 0)
			break;
	}
	
	close(from); // We always can close correct?
	close(to_the);	

	return 0;
}
