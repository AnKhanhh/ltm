#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX_CLIENT 10
#define MAX_MSG_LEN 1024

typedef struct client {
	int sock_fd;
	struct sockaddr_in addr;
	char id[20];
} client_t;

int main(int argc, char *argv[]) {
	if ( argc != 2 ) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
//	initialize server socket
	int server = socket(AF_INET, SOCK_STREAM, 0);
	if ( server < 0 ) {
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}
//	initialize port address for server
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(strtol(argv[1], NULL, 10));
//	bind socket to port and listen
	if ( bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
	if ( listen(server, MAX_CLIENT) < 0 ) {
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

//	initialize client list
	client_t clients[MAX_CLIENT];
	int client_count = 0;
//	initialize file descriptor set
	fd_set fds, fd_temp;
	FD_ZERO(&fds);
	FD_SET(server, &fds);

	while ( 1 ) {
//		reset fd set due to select() modifying input
		FD_COPY(&fds, &fd_temp);
		if ( client_count == 0 ) {
			printf("\nWaiting for client on %s:%s\n",
				   inet_ntoa(server_addr.sin_addr), argv[1]);
		}

		if ( select(FD_SETSIZE, &fd_temp, NULL, NULL, NULL) < 0 ) {
			perror("select() failed");
			continue;
		}

//		new connection event
		if ( FD_ISSET(server, &fd_temp)) {
			struct sockaddr_in client_addr;
			socklen_t client_addr_len = sizeof(client_addr);
			int client = accept(server, (struct sockaddr *) &client_addr, &client_addr_len);
			if ( client < 0 ) {
				perror("accept() failed");
				continue;
			}

			if ( client_count < MAX_CLIENT ) {
//			add client to file descriptor and client list
				FD_SET(client, &fds);

				clients[client_count].sock_fd = client;
				clients[client_count].addr = client_addr;
				strcpy(clients[client_count].id, "");
				client_count++;
				printf("Client from %s:%d connected\n",
					   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				char *question = "Enter your client_id:";
				if ( send(client, question, strlen(question), 0) < 0 ) {
					perror("send() failed");
					continue;
				}
			} else {
//				accept and immediately close connection
				close(client);
				printf("Maximum number of clients reached.\nClient %d disconnected\n", client);
			}
		}

//		event from client sockets
		for ( int i = 0; i < client_count; i++ ) {
//			receive message from client
			if ( FD_ISSET(clients[i].sock_fd, &fd_temp)) {
				char msg[MAX_MSG_LEN];
				int msg_len = recv(clients[i].sock_fd, msg, MAX_MSG_LEN, 0);

//				socket closed, remove client
				if ( msg_len <= 0 ) {
					FD_CLR(clients[i].sock_fd, &fds);
					printf("Client from %s:%d disconnected\n",
						   inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
					clients[i] = clients[--client_count];
					continue;
//				socket open, client not registered
				} else if ( strcmp(clients[i].id, "") == 0 ) {
					char id[20];
					msg[msg_len] = 0;
					sscanf(msg, "%[^ \t\n]", clients[i].id);
					printf("Client from %s:%d registered as %s\n", inet_ntoa(clients[i].addr.sin_addr),
						   ntohs(clients[i].addr.sin_port), clients[i].id);
					char *query = "You have successfully registered!\n";
					if ( send(clients[i].sock_fd, query, strlen(query), 0) < 0 ) {
						perror("send() failed");
						continue;
					}
//				client registered, broadcast to all other clients
				} else {
					msg[msg_len] = 0;
					time_t now = time(NULL);
					struct tm *t = localtime(&now);
					char time_str[22];
					strftime(time_str, MAX_MSG_LEN, "%Y/%m/%d %I:%M:%S%p", t);
					char message[MAX_MSG_LEN + 50];
					sprintf(message, "%s %s: %s", time_str, clients[i].id, msg);
					for ( int j = 0; j < client_count; j++ ) {
						if ( j == i ) continue;
						if ( send(clients[j].sock_fd, message, strlen(message), 0) < 0 ) {
							perror("send() failed");
						}


					}
				}
			}
		}
		if ( client_count == 0 ) break;
	}
	close(server);
	puts("Server closed!");

	return 0;
}