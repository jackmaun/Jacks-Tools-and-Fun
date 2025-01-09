#ifndef IRC_SERVER_H
#define IRC_SERVER_H

#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_CLIENTS 100
#define MAX_NICKNAME 32
#define MAX_USERNAME 32

typedef struct {
    int fd;
    SSL *ssl;
    char nickname[MAX_NICKNAME];
    char ip_addr[46];
    bool is_registered;
    char username[MAX_USERNAME];
} IRCClient;

typedef struct {
    int server_fd;
    int port;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS];
    IRCClient clients[MAX_CLIENTS];
    int client_count;
    SSL_CTX *ssl_ctx;
} IRCServer;

bool server_init(IRCServer *server, int port);
void server_run(IRCServer *server);
void server_cleanup(IRCServer *server);
void handle_new_client(IRCServer *server);
bool init_openssl();
void cleanup_openssl();
SSL_CTX *create_context();
bool configure_context(SSL_CTX *ctx);

int poll(struct pollfd *fds, unsigned int nfds, int timeout);

#endif