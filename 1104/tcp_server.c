#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> // close()


int main(int argc, char const *argv[]) {
	int port;
	char hello_file_name[256], client_file_name[256];
	if (argc != 4) {
		printf("wrong number of args\n");
		return 1;
	}

	char *a;
	port = (int) strtol(argv[1], &a, 10);
	strcpy(hello_file_name, argv[2]);
	strcpy(client_file_name, argv[3]);

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
	int accept_1 = accept(listener, (struct sockaddr *) &sck_addr_client, &sck_addr_client_len);

	if (accept_1 == -1) {
		perror("acctept() failed");
		return 1;
	}

	FILE *hello_file = fopen(hello_file_name, "rb");
	FILE *client_file = fopen(client_file_name, "wb");
	char buf[2048];

	while (!feof(hello_file)) {
		int fread_1 = fread(buf, 1, sizeof(buf), hello_file);
		buf[fread_1] = 0;
	}
	send(accept_1, buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	while (1) {
		int recv_1 = recv(accept_1, buf, sizeof(buf), 0);
		if (recv_1 == 1) break;
		buf[recv_1] = 0;
		fwrite(buf, 1, strlen(buf), client_file);
	}

	fclose(hello_file);
	fclose(client_file);
	close(listener);
	close(accept_1);
	return 0;
}

//chay server: ./ex2 8081 hello.dat client.dat
// hello.dat va client.dat dat o cung folder voi ex2
// chay client: nc localhost 8081
