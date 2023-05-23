#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CLIENT 1
#define STR_LEN 256

// <ip_addr> <port> <port_send>
int main(int argc, char *argv[]) {
//	create udp socket and server_socket address, bind and listen
	int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if ( server_socket < 0 ) {
		perror("socket() failed:");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;                    // IPv4
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);    // localhost
	server_addr.sin_port = htons(strtol(argv[2], NULL, 10));
	if ( bind(server_socket, (struct sockaddr *) &server_addr, sizeof server_addr) < 0 ) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
	if ( listen(server_socket, MAX_CLIENT) < 0 ) {
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

//	setup fie descriptors
	fd_set fds, fd_temp;
	FD_ZERO(&fds);
	FD_SET(server_socket, &fds);
	FD_SET(STDIN_FILENO, &fds);

	while ( 1 ) {
		FD_COPY(&fds,&fd_temp);
		if ( select(FD_SETSIZE, &fd_temp, NULL,NULL,NULL) <0 ){
			perror("select()");
			break;
		}

		if ( FD_ISSET(STDIN_FILENO, &fd_temp) ){
			char s[STR_LEN];
			fgets(s, sizeof s, stdin);
		}

		if ( 0 ) break;
	}

	return 0;
}