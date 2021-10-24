#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

int main() {
	FILE* potok = fopen("number.txt", "w");
	
	for(size_t i = 0; i < 100000000; i++) {
		fprintf(potok, "%ld ", i);
		if (i % 20 == 0) fprintf(potok, "\n");
	}
	
	fclose(potok);
	return 0;
}
