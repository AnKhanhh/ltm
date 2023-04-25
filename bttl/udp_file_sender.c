#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_LEN 128

// <addr> <port> <file>
int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("wrong args count");
		return 1;
	}

//	setup socket address
	struct sockaddr_in server_addr = {};    // effectively equivalent to memset 0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(strtol(argv[2], NULL, 10));

//	create udp socket
	int client = socket(AF_INET, SOCK_DGRAM, 0);
	if (client == -1) {
		perror("socket() failed");
		return 1;
	}

//	extract and send file name
	char file_name[BUF_LEN];
	strcpy(file_name, strrchr(argv[3], '\\') + 1);
	if(sendto(client, file_name, strlen(file_name), 0, (struct sockaddr *) &server_addr , sizeof server_addr) !=
	   strlen(file_name)){
		perror("sendto() failed");
		return 1;
	}

//	sending file content
	FILE *file = fopen(argv[3],"r+");
	char buffer[BUF_LEN];
	while(fgets(buffer, BUF_LEN, file) != NULL){
		if(sendto(client, buffer, strlen(buffer), 0, (struct sockaddr *) &server_addr , sizeof server_addr) !=
		   strlen(buffer)){
			perror("sendto() failed");
			return 1;
		}
		memset(buffer, 0, strlen(buffer));
	}

	fclose(file);
	close(client);

	return 0;
}
