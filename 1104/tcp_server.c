#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int port;
	char hello_file[256], rec_file[256];
	if (argc != 4) {
		printf("wrong number of args\n");
		return 1;
	}

	char *a;
	port = (int) strtol(argv[1], &a, 10);
	strcpy(hello_file, argv[2]);
	strcpy(rec_file, argv[3]);

	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(port);

	if (bind(listener, (struct sockaddr *) &addr, sizeof(addr))) {
		perror("bind() unsuccessfully");
		return 1;
	}
	if (listen(listener, 5)) {
		perror("listen() unsuccessfully");
		return 1;
	}

	struct sockaddr_in client_addr;
	int sck_addr_client_len = sizeof(client_addr);
	int client = accept(listener, (struct sockaddr *) &client_addr, &sck_addr_client_len);
	if (client == -1) {
		perror("accept() failed");
		return 1;
	}


	FILE *hello = fopen(hello_file, "rb");
	FILE *rec = fopen(rec_file, "wb");
	char buffer[2048];
	// doc tu file vao buffer
	while (!feof(hello)) {
		int end = (int) fread(buffer, 1, sizeof(buffer), hello);
		buffer[end] = 0;
	}
	send(client, buffer, strlen(buffer), 0);

	memset(buffer, 0, sizeof(buffer));
	while (1) {
		int end = recv(client, buffer, sizeof(buffer), 0);
		buffer[end] = 0;
		fwrite(buffer, 1, strlen(buffer), rec);
	}

	fclose(hello);
	fclose(rec);
	close(listener);
	close(client);
	free(a);
	return 0;
}
