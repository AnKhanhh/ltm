#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CLIENT 16
#define STR_LEN 256

char *standardize(char *s);

typedef struct client {
	int sock_fd;
	struct sockaddr_in addr;
} client_t;

// <port>
int main(int argc, char *argv[]) {
	if ( argc != 2 ) {
		puts("Wrong arg count!");
		exit(EXIT_FAILURE);
	}
//	initialize server socket/fd and address, bind and listen
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( server_fd < 0 ) {
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(strtol(argv[1], NULL, 10));
	if ( bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
	if ( listen(server_fd, MAX_CLIENT) < 0 ) {
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

//	initialize client list
	client_t clients[MAX_CLIENT];
	int client_count = 0;
//	initialize file descriptor set
	fd_set fds, fd_temp;
	FD_ZERO(&fds);
	FD_SET(server_fd, &fds);

	puts("waiting for client ...");
	while ( 1 ) {
		FD_COPY(&fds, &fd_temp);
		if ( select(FD_SETSIZE, &fd_temp, NULL, NULL, NULL) < 0 ) {
			perror("select() failed");
			continue;
		}

//		connect to client if not over limit
		if ( FD_ISSET(server_fd, &fd_temp)) {
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
			if ( client_fd < 0 ) {
				perror("accept() failed");
				continue;
			}
			if ( client_count < MAX_CLIENT ) {
				FD_SET(client_fd, &fds);
				clients[client_count].sock_fd = client_fd;
				clients[client_count].addr = client_addr;
				client_count++;

				printf("Client from %s:%d connected\n",
					   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				char query[STR_LEN];
				snprintf(query, sizeof query, "Hi!, %d clients connecting right now.\n", client_count);
				if ( send(client_fd, query, strlen(query), 0) < 0 ) {
					perror("send() failed");
					continue;
				}
			} else {
//				close connection
				close(client_fd);
				printf("Maximum number of clients reached.\nClient %d disconnected\n", client_fd);
			}
		}

		for ( int i = 0; i < client_count; i++ ) {
			if ( FD_ISSET(clients[i].sock_fd, &fd_temp)) {
				char msg[STR_LEN];
				long msg_len = recv(clients[i].sock_fd, msg, STR_LEN, 0);
//				socket closed, remove client
				if ( msg_len <= 0 || strcmp(msg,"exit") == 0) {
					FD_CLR(clients[i].sock_fd, &fds);
					close(clients[i].sock_fd);
					printf("Client from %s:%d disconnected\n",
						   inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
					clients[i] = clients[--client_count];
					continue;
//				client send msg, standardize
				} else {
					msg[msg_len] = 0;
					char *query = standardize(msg);
					if ( send(clients[i].sock_fd, query, strlen(query), 0) < 0 ) {
						perror("send() failed");
					}
					free(query);
				}
			}
		}
		if ( client_count == 0 ) {
			puts("All client disconnected, session ends.");
			break;
		}
	}

	close(server_fd);
	return 0;
}

char *standardize(char *s) {
	long ss_len = (long) strlen(s);
	char *ss = s;
	while ( isspace(*ss)) {
		ss++;
		ss_len--;
	}

	char *s_result = malloc(sizeof(char) * ss_len + 5);
	long i = 0;
	s_result[i++] = '\"';
	char *tk = strtok(ss, " \t");
	while ( tk != NULL) {
		tk[0] = toupper(tk[0]);
		strcpy(&s_result[i], tk);
		i += strlen(tk);
		s_result[i++] = ' ';
		tk = strtok(NULL, " \t");
	}

	long sr_len = (long) strlen(s_result);
	while ( isspace(s_result[sr_len-1])){
		sr_len--;
	}

	strcpy(&s_result[sr_len], "\"\n");
	return s_result;
}
