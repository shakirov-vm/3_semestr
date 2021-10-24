#include <stdlib.h>
#include <stdio.h>

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
		
	FILE* from = fopen(argv[1], "r");
	if (from == NULL) {
		printf("FROM file can't be open\n");
		return 2;
	} 
	FILE* to_the = fopen(argv[2], "w");
	if (to_the == NULL) {
		printf("TO_THE file can't be open\n");
		return 3;
	} 
	
	size_t readed;
	size_t writed;
	
	while(1) {
		readed = 0;
		writed = 0;
		
		readed = fread(buffer, sizeof(char), megabyte, from); // feof need
		
		if (!feof(from)) {
			if (readed != megabyte) {
			printf("We can't read file, %lu readed\n", readed);
			
			fclose(from); // We always can close correct?
			fclose(to_the);
			return 4;
			}
		}
		
		writed = fwrite(buffer, sizeof(char), readed, to_the); 
		
		if (writed != readed) {
			printf("We can't write correctly, %lu writed\n", writed);
			fclose(from); // We always can close correct?
			fclose(to_the);
			return 5;
		}
		
		if (feof(from)) break;
	}
	
	fclose(from); // We always can close correct?
	fclose(to_the);	

	return 0;
}
