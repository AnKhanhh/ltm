#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <dirent.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 9000
#define PATH_TO_DIR "C:/Main/netcat-win32-1.12"
#define STR_LEN 1024

// dirent?

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	server_addr.sin_port = htons(SERVER_PORT);

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if ( server_fd < 0 ) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	if ( bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0 ) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if ( listen(server_fd, 1) < 0 ) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	DIR *dir = opendir(PATH_TO_DIR);
	struct dirent *entry;

	int file_count = 0;
	char *file_list = malloc(1);
	while (( entry = readdir(dir)) != NULL) {
		if ( entry->d_type == 8 ) {
			file_list = realloc(file_list, strlen(file_list) + strlen(entry->d_name) + 3);
			strcat(file_list, entry->d_name);
			strcat(file_list, "\r\n");
			entities_count++;
		}
	}

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if ( client_fd < 0 ) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	if ( fork() == 0 ) {
		exit(EXIT_SUCCESS);
	} else {
		char query[STR_LEN];
		while ( 1 ) {
			int query_len =  recv(client_fd, query, STR_LEN, 0);
			if ( query_len <= 0 ) {
				perror("recv");
				close(client_fd);
			}
		}
	}

	closedir(dir);
	close(server_fd);
	return 0;
}
