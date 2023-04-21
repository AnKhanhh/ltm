#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define STR_LENGTH 1024

void fgets_wrapper(char *s, int s_size);


int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("wrong args number");
		return 1;
	}

//	setup socket address
	struct sockaddr_in server_addr = {};    // effectively equivalent to memset 0
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(strtol(argv[2], NULL, 10));

//	create and connect socket
	int client = socket(AF_INET, SOCK_STREAM, 0);
	if (client == -1) {
		perror("socket() failed");
		return 1;
	}
	if (connect(client, (struct sockaddr *) &server_addr, sizeof server_addr) == -1) {
		perror("connect() failed");
		return 1;
	}

	char buffer[STR_LENGTH];
//	taking in relevant information
	printf("input device name: ");
	char device[STR_LENGTH];
	fgets_wrapper(device, sizeof device);
	printf("input number of disks: ");
	fgets_wrapper(buffer, sizeof device);
	long number = strtol(buffer, NULL, 10);
	char disk_name[number][STR_LENGTH];
	short disk_size[number];
	for (int i = 0; i < number; ++i) {
		printf("input name of disk %d: ", i + 1);
		fgets_wrapper(disk_name[i], sizeof disk_name[i]);
		printf("input size of disk %d: ", i + 1);
		fgets_wrapper(buffer, sizeof buffer);
		disk_size[i] = strtol(buffer, NULL, 10);
	}

//	packaging and sending information
	char package[STR_LENGTH];
	int packaged = snprintf(package, STR_LENGTH, "%s|%ld", device, number);        // package size excluding null
	for (int i = 0; i < number && packaged < STR_LENGTH; i++) {
		packaged += snprintf(package + packaged, STR_LENGTH, "|%s|%hi", disk_name[i], disk_size[i]);
	}


	return 0;
}

void fgets_wrapper(char *s, int s_size) {
	if ((fgets(s, s_size, stdin) != NULL)) {
		s[strcspn(s, "\r\n")] = 0;
		return;
	}
	puts("error while parsing input");
}
