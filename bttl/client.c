#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_LEN 8

void fgets_wrapper(char *s, int s_size);

// <ip addr> <port> <file name>
int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("wrong args count!");
		return 1;
	}

	//	setup socket address
	struct sockaddr_in server_addr = {};    // effectively equivalent to memset 0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(strtol(argv[2], NULL, 10));

//	create and connect socket
	int client = socket(AF_INET, SOCK_STREAM, 0);
	if (client == -1) {
		perror("socket() failed");
		return 1;
	}
	if (connect(client, (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
		perror("connect() failed");
		return 1;
	}

//	read file chunk by chunk instead of read the whole file at once
	char buffer[BUF_LEN];
	int parsed;
	FILE *file = fopen(argv[3], "r+");
	do {
		memset(buffer,0,BUF_LEN);
		parsed = fread(buffer, 1, BUF_LEN - 1, file);
		if (send(client, buffer, sizeof(buffer) -1, 0) == -1) {
			perror("send() failed");
			return 1;
		}
	} while (strlen(buffer) == BUF_LEN - 1);

	return 0;
}

void fgets_wrapper(char *s, int s_size) {
	if ((fgets(s, s_size, stdin) != NULL)) {
		s[strcspn(s, "\r\n")] = 0;
		return;
	}
	puts("error while parsing input");
}
