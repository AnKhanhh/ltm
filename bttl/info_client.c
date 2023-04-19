#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void fgets_wrapper(char*, int);

int main(int argc, char *argv[]){
	char buffer[256];
	char* x;

	printf("ten may");
	char comp[256];
	fgets_wrapper(comp, sizeof comp);
	printf("so o dia");
	fgets_wrapper(comp, sizeof comp);
	long num = strtol(comp, &x, 10);
	for (int i = 0; i < num; ++i) {
		printf("iter");
	}
	
    return 0;
}

void fgets_wrapper(char* s, int s_size){
	if((fgets(s, s_size, stdin) != NULL)) {
		s[strcspn(s, "\r\n")] = 0;
		return;
	}
}