#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define NUMBER_THREADS 8
#define NUMBER_CLIENTS 10

void *client_proc(void *);

int main(int argc, char *argv[]) {
	if ( argc != 2 ) {
		printf("wrong args count");
		exit(EXIT_FAILURE);
	}

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( server_fd < 0 ) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));
	if ( bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if ( listen(server_fd, NUMBER_CLIENTS) < 0 ) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	printf("Listening on %s:%d ...\n",
		   inet_ntoa(server_addr.sin_addr),
		   ntohs(server_addr.sin_port));

	pthread_t thread_id;
	for ( int i = 0; i < NUMBER_THREADS; i++ ) {
		if ( pthread_create(&thread_id, NULL, client_proc, &server_fd) != 0 ) {
			perror("pthread_create() failed");
			sched_yield();
		}
	}

	pthread_join(thread_id, NULL);

	getchar();
	close(server_fd);

	return 0;
}

void *client_proc(void *param) {
	int server = *(int *) param;
	char buf[BUF_SIZE];

	while ( 1 ) {
		struct sockaddr_in client_addr = {};
		socklen_t client_addr_len = sizeof(client_addr);
		int client = accept(server, (struct sockaddr *) &client_addr, &client_addr_len);
		if ( client < 0 ) {
			perror("accept() failed");
			exit(EXIT_FAILURE);
		}
		printf("New client from %s:%d accepted in thread %ld with id %d\n",
			   inet_ntoa(client_addr.sin_addr),
			   ntohs(client_addr.sin_port), (long) pthread_self(), getpid());

		long len = recv(client, buf, BUF_SIZE, 0);
		if ( len < 0 ) {
			perror("recv");
			continue;
		} else if ( len == 0 ) {
			printf("Client from %s:%d disconnected\n",
				   inet_ntoa(client_addr.sin_addr),
				   ntohs(client_addr.sin_port));
			close(client);
			continue;
		} else {
			buf[strcspn(buf, "\n")] = 0;
			printf("Received %ld bytes from client %d: %s\n", len, client, buf);
			char msg[BUF_SIZE];
			fgets(msg, sizeof msg, stdin);
			if ( send(client, msg, strlen(msg), 0) < 0 ) {
				perror("send() failed");
				exit(EXIT_FAILURE);
			}
		}

		printf("Client from %s:%d disconnected\n",
			   inet_ntoa(client_addr.sin_addr),
			   ntohs(client_addr.sin_port));
		close(client);
	}

	return NULL;
}