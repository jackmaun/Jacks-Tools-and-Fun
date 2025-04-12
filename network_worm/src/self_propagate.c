#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 1234 //arbitrary port number

void send_file(const char *ip, const char *filename) {
	int sock;
	struct sockaddr_in server;
	FILE *file;
	char buffer[1024];
	int bytes_read;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		perror("[!] Can't create socket");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	inet_pton(AF_INET, ip, &server.sin_addr);

	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
		perror("[!] Connection failed.");
		close(sock);
		exit(1);
	}

	file = fopen(filename, "rb");
	if(file == NULL) {
		perror("[!] File not found");
		close(sock);
		exit(1);
	}

	while((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		if(send(sock, buffer, bytes_read, 0) == -1) {
			perror("[!] File transfer failed");
			fclose(file);
			close(sock);
			exit(1);
		}
	}

	prinf("[+] File transfer complete");
	
	fclose(file);
	close(sock);
}

