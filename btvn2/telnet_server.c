#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX_CLIENT 10
#define MSG_LEN 1024


typedef struct client {
	int sock_fd;
	struct sockaddr_in addr;
	char user_name[16];
	char pswd[16];
} client_t;

int main(int argc, char *argv[]) {
	if ( argc != 4 ) {
		printf("Usage: %s <port> <path_to_dtb> <path_to_out>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
//	initialize server socket and broadcast port
	int server = socket(AF_INET, SOCK_STREAM, 0);
	if ( server < 0 ) {
		perror("socket() failed");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(strtol(argv[1], NULL, 10));
//	bind server socket to port and listen
	if ( bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("bind() failed");
		exit(EXIT_FAILURE);
	}
	if ( listen(server, MAX_CLIENT) < 0 ) {
		perror("listen() failed");
		exit(EXIT_FAILURE);
	}

//	open dtb for reading
	FILE *dtb = fopen(argv[2], "r");
	if ( dtb == NULL) {
		perror("fopen() failed");
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
				strcpy(clients[client_count].user_name, "");
				strcpy(clients[client_count++].pswd, "");
				printf("Client from %s:%d connected\n",
					   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				char *question = "Enter your user name: ";
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
				char msg[MSG_LEN];
				long msg_len = recv(clients[i].sock_fd, msg, MSG_LEN, 0);

//				socket closed, remove client
				if ( msg_len <= 0 ) {
					FD_CLR(clients[i].sock_fd, &fds);
					printf("Client from %s:%d disconnected\n",
						   inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
					clients[i] = clients[--client_count];
					continue;
//				socket open, password not provided
				} else if ( strcmp(clients[i].user_name, "") == 0 ) {
					msg[msg_len] = 0;
					sscanf(msg, "%[^ \t\n]", clients[i].user_name);
					printf("Client from %s:%d is now named %s\n", inet_ntoa(clients[i].addr.sin_addr),
						   ntohs(clients[i].addr.sin_port), clients[i].user_name);
					char *query = "Enter your password: ";
					if ( send(clients[i].sock_fd, query, strlen(query), 0) < 0 ) {
						perror("send() failed");
						continue;
					}
//				client not registered, match names and passwords
				} else if ( strcmp(clients[i].pswd, "") == 0 ) {
					msg[msg_len] = 0;
					sscanf(msg, "%[^ \t\n]", clients[i].pswd);
					char line[20];
					char *temp = NULL;
					size_t match_found = 0;
					while ( fgets(line, 20, dtb) != NULL) {
						temp = strtok(line, " ");
						if ( strcmp(clients[i].user_name, temp) == 0 ) {
							temp = strtok(NULL, "\r\n");
							if ( strcmp(clients[i].pswd, temp) == 0 ) {
								match_found++;
								char *query = "Client registered! You can now issue commands!\n";
								if ( send(clients[i].sock_fd, query, strlen(query), 0) < 0 ) {
									perror("send() failed");
								}
								break;
							}
						}
					}
					if ( !match_found ) {
						strcpy(clients[i].user_name, "");
						strcpy(clients[i].pswd, "");
						char *query = "No match found. Enter your user name again: ";
						if ( send(clients[i].sock_fd, query, strlen(query), 0) < 0 ) {
							perror("send() failed");
							continue;
						}
					}
//				client registered, execute command
				} else {
					msg[msg_len - 1] = 0;
					char cmd[256];
					snprintf(cmd, sizeof cmd, "%s > %s", msg, argv[3]);
					printf("issuing command: %s.", cmd);
					if ( system(cmd) != 1 ) {
						char out_msg[MSG_LEN];
						FILE *f_out = fopen(argv[3], "r");
						while ( fgets(out_msg, (int) sizeof(out_msg), f_out) != NULL) {
							if ( send(clients[i].sock_fd, out_msg, strlen(out_msg), 0) < 0 ) perror("send() failed");
						}
						fclose(f_out);
					} else{
						char out_msg[MSG_LEN] = "wrong command";
						if ( send(clients[i].sock_fd, out_msg, strlen(out_msg), 0) < 0 ) perror("send() failed");
					}
				}
			}
		}
		if ( client_count == 0 ) break;
	}

	fclose(dtb);
	close(server);
	puts("Server and DTB closed. Session ended!");

	return 0;
}
