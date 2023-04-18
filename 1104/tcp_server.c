#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char const *argv[]) {
	int port;
	char f_hello[256], f_rec[256];
	if (argc != 4) {
		printf("wrong number of args\n");
		return 1;
	}

	char *a;
	port = (int) strtol(argv[1], &a, 10);
	strcpy(f_hello, argv[2]);
	strcpy(f_rec, argv[3]);

	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in sck_addr;
	sck_addr.sin_family = AF_INET;
	sck_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sck_addr.sin_port = htons(port);

	if (bind(listener, (struct sockaddr *) &sck_addr, sizeof(sck_addr))) {
		perror("bind() failed");
		return 1;
	}
	if (listen(listener, 5)) {
		perror("listen() failed");
		return 1;
	}

	struct sockaddr_in sck_addr_client;
	int sck_addr_client_len = sizeof(sck_addr_client);
	int client = accept(listener, (struct sockaddr *) &sck_addr_client, &sck_addr_client_len);

	if (client == -1) {
		perror("accept() failed");
		return 1;
	}

	FILE *hello_file;
	if ((hello_file = fopen(f_hello, "w+")) == NULL) return 1;
		FILE *client_file = fopen(f_rec, "w+");
	char buf[2048];

	while (!feof(hello_file)) {
		int end = (int) fread(buf, 1, sizeof(buf), hello_file);
		buf[end] = 0;
	}
	puts(buf);
//	send(client, buf, strlen(buf), 0);

//	memset(buf, 0, sizeof(buf));
//	while (1) {
//		int end = (int) recv(client, buf, sizeof(buf), 0);
//		if (end <= 0) break;
//		buf[end] = 0;
//		fwrite(buf, 1, strlen(buf), client_file);
//	}

	fclose(hello_file);
	fclose(client_file);
	close(listener);
	close(client);
	return 0;
}
