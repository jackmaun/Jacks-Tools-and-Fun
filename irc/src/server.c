#include "../include/server.h"
#include "../include/handle_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdbool.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define close(fd) closesocket(fd)
#define inet_ntop(af, src, dst, size) InetNtopA(af, src, dst, size)
#define perror(msg) fprintf(stderr, "%s: %d\n", msg, WSAGetLastError())

bool init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    return true;
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_context() {
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if(!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return ctx;
}

bool configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    SSL_CTX_set_verify_depth(ctx, 1);

    if(SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if(SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if(!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate\n");
        return false;
    }

    return true;
}

bool server_init(IRCServer *server, int port) {
    server->port = port;
    server->client_count = 0;

    if(!init_openssl()) {
        fprintf(stderr, "Failed to initialize OpenSSL\n");
        return false;
    }

    server->ssl_ctx = create_context();
    if(!server->ssl_ctx) {
        fprintf(stderr, "Failed to create SSL context\n");
        return false;
    }

    if(!configure_context(server->ssl_ctx)) {
        fprintf(stderr, "Failed to configure SSL context\n");
        return false;
    }

    if((server->server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return false;
    }
    
    int opt = 1;
    if(setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server->server_fd);
        return false;
    }

    memset(&server->server_addr, 0, sizeof(server->server_addr));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server->server_addr.sin_port = htons(port);

    if(bind(server->server_fd, (struct sockaddr *)&server->server_addr, sizeof(server->server_addr)) == -1) {
        perror("Bind failed");
        close(server->server_fd);
        return false;
    }

    if(listen(server->server_fd, 5) == -1) {
        perror("Listen failed");
        close(server->server_fd);
        return false;
    }

    for(int i = 0; i < MAX_CLIENTS; i++) {
        server->fds[i].fd = -1;
        server->clients[i].ssl = NULL;
        memset(server->clients[i].nickname, 0, MAX_NICKNAME);
        memset(server->clients[i].ip_addr, 0, INET_ADDRSTRLEN);
    }
    server->fds[0].fd = server->server_fd;
    server->fds[0].events = POLLIN;

    printf("[Server] Started on port %d (SSL enabled)\n", port);
    return true;
}

void handle_new_client(IRCServer *server) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_len);

    if(client_fd == -1) {
        perror("Accept failed");
        return;
    }

    if(server->client_count >= MAX_CLIENTS) {
        close(client_fd);
        return;
    }

    SSL *ssl = SSL_new(server->ssl_ctx);
    SSL_set_fd(ssl, client_fd);

    if(SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(client_fd);
        return;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), ip, INET_ADDRSTRLEN);

    server->fds[server->client_count + 1].fd = client_fd;
    server->fds[server->client_count + 1].events = POLLIN;
    server->clients[server->client_count].fd = client_fd;
    server->clients[server->client_count].ssl = ssl;
    strncpy(server->clients[server->client_count].ip_addr, ip, INET_ADDRSTRLEN - 1);
    snprintf(server->clients[server->client_count].nickname, MAX_NICKNAME, "user%d", client_fd);

    printf("[Server] New SSL connection from %s (FD: %d)\n", ip, client_fd);
    server->client_count++;

    char msg[512];
    snprintf(msg, sizeof(msg), "Welcome! Use /NICK <nickname> to set your nickname\r\n");
    SSL_write(ssl, msg, strlen(msg));
}

void server_run(IRCServer *server) {
    while(1) {
        int poll_count = poll(server->fds, server->client_count + 1, -1);
        
        if(poll_count == -1) {
            perror("poll failed");
            break;
        }

        for (int i = 0; i < server->client_count + 1; i++) {
            if(server->fds[i].revents & POLLIN) {
                if((intptr_t)server->fds[i].fd == (intptr_t)server->server_fd) {
                    handle_new_client(server);
                }
                else {
                    handle_client_message(&server->fds[i], server);
                }
            }
        }
    }
}

void server_cleanup(IRCServer *server) {
    for(int i = 0; i < server->client_count; i++) {
        if(server->clients[i].ssl) {
            SSL_shutdown(server->clients[i].ssl);
            SSL_free(server->clients[i].ssl);
        }
        if(server->clients[i].fd != -1) {
            close(server->clients[i].fd);
        }
    }
    
    close(server->server_fd);
    SSL_CTX_free(server->ssl_ctx);
    cleanup_openssl();
    printf("[Server] Shutting down\n");
}