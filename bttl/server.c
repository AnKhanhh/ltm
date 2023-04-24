#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#define BUF_LEN 1024

void fgets_wrapper(char *s, int s_size);

// <ip addr> <port>
int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("wrong args count");
		return 1;
	}

//	setup socket address
	int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); // bind to specified address only
	//htonl(INADDR_ANY); - bind to all interfaces
	server_addr.sin_port = htons((int) strtol(argv[2], NULL, 10));

//	bind and listen to port
	if ((bind(server, (struct sockaddr *) &server_addr, sizeof server_addr)) == -1) {
		perror("bind() failed");
		return 1;
	}
	if ((listen(server, 5) == -1)) {
		perror("listen() failed");
		return 1;
	}

//	accept client
	struct sockaddr_in client_addr = {};
	int c_addr_len = sizeof client_addr;
	int client = accept(server, (struct sockaddr *) &client_addr, &c_addr_len);
	if (client == -1) {
		perror("accept() failed");
		return 1;
	}

//	receive all data before handling
	char buffer[BUF_LEN];
	memset(buffer, 0, BUF_LEN);
	long received, offset = 0;
	do {
		received = recv(client, buffer + offset, sizeof buffer, 0);
		offset += received;
		if (received < 0) {
			perror("recv() failed");
			return 1;
		}
	} while (received != 0);
	puts(buffer);


	return 0;
}
