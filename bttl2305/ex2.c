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
	if ( argc != 4 ) {
		printf("wrong args count");
		exit(EXIT_FAILURE);
	}

//	create udp socket/fd and udp address, bind
//	note: udp connection cant listen()
	int host_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if ( host_fd < 0 ) {
		perror("socket() failed:");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in host_addr = {};
	host_addr.sin_family = AF_INET;                    // IPv4
	host_addr.sin_addr.s_addr = inet_addr(argv[1]);    // localhost
	host_addr.sin_port = htons(strtol(argv[2], NULL, 10));
	if ( bind(host_fd, (struct sockaddr *) &host_addr, sizeof host_addr) < 0 ) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
//	target udp address
	struct sockaddr_in target_addr = {};
	target_addr.sin_family = AF_INET;                    // IPv4
	target_addr.sin_addr.s_addr = INADDR_ANY;
	target_addr.sin_port = htons(strtol(argv[3], NULL, 10));

//	setup fie descriptors
	fd_set fds, fd_temp;
	FD_ZERO(&fds);
	FD_SET(host_fd, &fds);
	FD_SET(STDIN_FILENO, &fds);

	while ( 1 ) {
		FD_COPY(&fds, &fd_temp);
		if ( select(FD_SETSIZE, &fd_temp, NULL, NULL, NULL) < 0 ) {
			perror("select()");
			break;
		}

		if ( FD_ISSET(STDIN_FILENO, &fd_temp)) {
			char s[STR_LEN];
			fgets(s, sizeof s, stdin);
			if ( sendto(host_fd, s, strlen(s), 0, (struct sockaddr *) &target_addr, sizeof target_addr) < 0 ) {
				perror("sendto()");
				exit(EXIT_FAILURE);
			}
		}

		if ( FD_ISSET(host_fd, &fd_temp)) {
			char msg[STR_LEN];
			struct sockaddr_in sender_addr;
			memset(&sender_addr, 0, sizeof(sender_addr));
			socklen_t sender_addr_len = sizeof(sender_addr);
			long msg_len = recvfrom(host_fd, msg, STR_LEN, 0, NULL, NULL);
			if ( msg_len < 0 ) {
				perror("recvfrom()");
				exit(EXIT_FAILURE);
			}
			msg[msg_len] = 0;
			if ( strcmp(msg, "exit\n") == 0 ) {
				puts("socket closing ...");
				break;
			} else { printf("received:%s", msg); }

		}

	}

	close(host_fd);
	return 0;
}