#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
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



//	struct sockaddr_in client_addr;
//	socklen_t client_addr_len = sizeof(client_addr);
//	pass pointers if info on client is needed
	int client_fd = accept(server_fd, NULL, NULL);
	if ( client_fd < 0 ) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	if ( fork() == 0 ) {
		DIR *dir = opendir(PATH_TO_DIR);
		struct dirent *entry;
		//	count number of files
		int file_count = 0;
		char *file_list = malloc(5);
		while (( entry = readdir(dir)) != NULL) {
			if ( entry->d_type == DT_REG ) {
				file_list = realloc(file_list, strlen(file_list) + strlen(entry->d_name) + 3);
				strcat(file_list, entry->d_name);
				strcat(file_list, "\r\n");
				file_count++;
			}
		}
		strcat(file_list,"\r\n\r\n");
		if ( file_count == 0 ) {
			if ( send(client_fd, "ERR no file", STR_LEN, 0) < 0 ) {
				perror("send");
			}
		} else {
			char msg[STR_LEN];
			snprintf(msg, STR_LEN, "OK %d\r\n", file_count);
			if ( send(client_fd, msg, strlen(msg), 0) < 0 ) {
				perror("send");
			}
			if ( send(client_fd, file_list, strlen(file_list), 0) < 0 ) {
				perror("send");
			}
		}

		close(server_fd);
		close(client_fd);
		free(file_list);
		exit(EXIT_SUCCESS);


	} else {
		char query[STR_LEN];
		while ( 1 ) {
			ssize_t query_len = recv(client_fd, query, STR_LEN, 0);
			if ( query_len <= 0 ) {
				perror("recv");
				close(client_fd);
				break;
			}
		}
	close(server_fd);
	close(client_fd);
	}

	return 0;
}
