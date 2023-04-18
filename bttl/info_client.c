#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char* fgets_wrapper(char*, int);

int main(int argc, char *argv[]){
	char buffer[256];
	puts(fgets_wrapper(buffer, sizeof buffer));
    return 0;
}

char* fgets_wrapper(char* s, int s_size){
	if((fgets(s, s_size, stdin) != NULL)) {
		s[strcspn(s, "\r\n")] = 0;
		return s;
	}
}