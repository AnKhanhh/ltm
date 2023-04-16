#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	if(argc != 3){
		printf("wrong number of args\n");
		return 1;
	}

	char *a, *ip = argv[1];
	int port = (int) strtol(argv[2], &a, 10); // 80 = http, 443 = https
	free(a);

	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	int ret = connect(client, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) {
		perror("connect() unsuccessfully");
		return 1;
	}

	char msg[256];
	fgets(msg, sizeof(msg), stdin);
	send(client, msg, strlen(msg), 0);

	free(ip);
//	close(client);
	return 0;
}