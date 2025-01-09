#include "../include/server.h"

#include <stdio.h>

#include <winsock2.h>

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    IRCServer server;

    if(!server_init(&server, 6667)) {
        fprintf(stderr, "Failed to init\n");
        WSACleanup();
        return 1;
    }
    server_run(&server);
    server_cleanup(&server);

    WSACleanup();
    return 0;
}