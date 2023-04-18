#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

	int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	addr.sin_port = htons(9000);

	int ret = connect(client, (struct sockaddr *) &addr, sizeof(addr));
	if (ret == -1) {
		perror("connect() unsuccessfully");
		return 1;
	}
	return 0;
}
