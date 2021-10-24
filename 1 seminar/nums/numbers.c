#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv) {
//Проверка на переплнение?
	if(argc != 2) {
	printf("You enter %d args. Need 2 arguments\n", argc);
	return 2;
	}
	//printf("[%s]\n", argv[1]);
	
	char* num_end;
	long biggest = strtol(argv[1], &num_end, 0);    //x86
	if (errno == ERANGE) {
	printf("Warning: your number bigger then long\n");
	return 3;
	}

	if (num_end != NULL && *num_end/* != ''*/) { // ?
	printf("Incorrect enter\n");
	return 4;
	}
	
	if (biggest <= 0) {
	printf("Your number less then zero or not number\n");
	return 1;
	}
	
	for(size_t i = 0; i < biggest; i++) {
	printf("%lu ", i);
	}
	
	printf("\n");
	return 0;
}
