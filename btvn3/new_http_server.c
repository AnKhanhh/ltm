// server that accepts connection a number of times
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_SESSION 4
#define STR_LEN 1024
#define SERVER_PORT 9000

int main(int argc, char *argv[]) {

	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( server_fd < 0 ) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	if ( bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
	if ( listen(server_fd, MAX_SESSION) < 0 ) {
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

	for ( int i = 0; i < MAX_SESSION; ++i ) {
		if ( fork() == 0 ) {
			struct sockaddr_in client_addr = {};
			socklen_t client_addr_len = sizeof(client_addr);
			int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
			if ( client_fd < 0 ) {
				perror("accept");
				exit(EXIT_FAILURE);
			}
			printf("client %s:%d accepted to fd %d, process %d\n",
				   inet_ntoa(client_addr.sin_addr),
				   ntohs(client_addr.sin_port), client_fd, getpid());
			ssize_t len;
			do {
				char msg[STR_LEN];
				len = recv(client_fd, msg, STR_LEN, 0);
				msg[len] = 0;
				printf("%ld bytes received: %s", (long) len, msg);
			} while ( len > 0 );
			close(client_fd);
			close(server_fd);
			exit(EXIT_SUCCESS);
		} else {
			wait(NULL);
		}
	}

	close(server_fd);
	return 0;
}