#include <stdlib.h>
#include <stdio.h>

int main(size_t argc, char** argv) {
	printf(">> %s\n", argv[1]);
	FILE* potok = fopen(argv[1], "r");
	printf("potok is %p\n", potok);

	char* buf = (char*) calloc (3, 1);
	printf("buf before read is [%s], %p\n", buf, buf);
	int readed = fread(buf, 1, 3, potok);
	printf("read - %d\n", readed);
	printf("<< %s\n", buf); 
	return 0;
}