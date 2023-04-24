#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char const *argv[]) {
	if (argc != 4) {
		printf("wrong number of args\n");
		return 1;
	}

//	create socket
	int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	setup socket
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // bind to all interfaces
	//  inet_addr("127.0.0.1") - bind to localhost only
	server_addr.sin_port = htons((int) strtol(argv[1], NULL, 10));

	if (bind(server, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
		perror("bind() failed");
		return 1;
	}
	if (listen(server, 5) == -1) {    // max client allowed 5
		perror("listen() failed");
		return 1;
	}

//	accept connection from client
	struct sockaddr_in client_addr;
	int c_addr_len = sizeof(client_addr);
	int client = accept(server, (struct sockaddr *) &client_addr, &c_addr_len);
	if (client == -1) {
		perror("accept() failed");
		return 1;
	}

//	send hello from file to client
	char greetings[256];
	long length;
	FILE *hello_file = fopen(argv[2], "r+");
	if (hello_file == NULL) {
		perror("fopen() failed");
		return 1;
	}
	fseek(hello_file, 0,SEEK_END);
	length = ftell(hello_file);
	fseek(hello_file, 0 , SEEK_SET);
	fread(greetings, 1, length, hello_file);
	greetings[length] = 0;
	if(send(client, greetings, strlen(greetings),0) == -1){
		perror("send() failed");
		return 1;
	}
	fclose(hello_file);

	char record[1024];
	FILE *client_file = fopen(argv[3], "a");
	while (1) {
		long received =  recv(client, record, sizeof(record), 0);
		if (received < 0){
			perror("recv() failed");
			return 1;
		} else if(received == 0){
			break;
		}
		fwrite(record, 1, received, client_file);
	}
	fclose(client_file);

	close(server);
	close(client);
	return 0;
}



