#include "../include/handle_client.h"
#include "../include/message.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

static void format_time(char *time_str, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_str, size, "%H:%M:%S", tm_info);
}

static void broadcast_message(IRCServer *server, int sender_fd __attribute__((unused)), const char *message) {
    printf("%s", message);
    for(int i = 0; i < server->client_count; i++) {
        if(server->clients[i].fd != -1 && 
           server->clients[i].fd != server->server_fd && 
           server->clients[i].ssl != NULL) {
            SSL_write(server->clients[i].ssl, message, strlen(message));
        }
    }
}

static char *get_client_nickname(IRCServer *server, int fd) {
    for(int i = 0; i < server->client_count; i++) {
        if(server->clients[i].fd == fd) {
            return server->clients[i].nickname;
        }
    }
    return "anonymous";
}

static SSL *get_client_ssl(IRCServer *server, int fd) {
    for(int i = 0; i < server->client_count; i++) {
        if(server->clients[i].fd == fd) {
            return server->clients[i].ssl;
        }
    }
    return NULL;
}

static void handle_nick_command(IRCServer *server, int fd, const char *params) {
    if(!params || strlen(params) == 0) return;
    
    while(isspace(*params)) params++;
    
    char new_nick[MAX_NICKNAME];
    strncpy(new_nick, params, MAX_NICKNAME - 1);
    new_nick[MAX_NICKNAME - 1] = '\0';

    int len = strlen(new_nick);
    while(len > 0 && (isspace(new_nick[len-1]) || new_nick[len-1] == '\n' || new_nick[len-1] == '\r')) {
        new_nick[--len] = '\0';
    }

    for(int i = 0; i < server->client_count; i++) {
        if(server->clients[i].fd == fd) {
            char time_str[9];
            char msg[512];
            format_time(time_str, sizeof(time_str));
            
            if(strlen(server->clients[i].nickname) > 0) {
                snprintf(msg, sizeof(msg), "%s changed nick to %s - %s\r\n", 
                    server->clients[i].nickname, new_nick, time_str);
                broadcast_message(server, fd, msg);
            }
            
            strncpy(server->clients[i].nickname, new_nick, MAX_NICKNAME - 1);
            server->clients[i].nickname[MAX_NICKNAME - 1] = '\0';
            break;
        }
    }
}

void handle_client_message(struct pollfd *client_fd, IRCServer *server) {
    char buffer[512];
    SSL *ssl = get_client_ssl(server, client_fd->fd);
    if(!ssl) return;

    int bytes_read = SSL_read(ssl, buffer, sizeof(buffer) - 1);

    if(bytes_read <= 0) {
        int err = SSL_get_error(ssl, bytes_read);
        if(err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL) {
            char time_str[9];
            format_time(time_str, sizeof(time_str));
            
            char msg[512];
            snprintf(msg, sizeof(msg), "%s disconnected - %s\r\n", 
                get_client_nickname(server, client_fd->fd), time_str);
            broadcast_message(server, client_fd->fd, msg);
            
            for(int i = 0; i < server->client_count; i++) {
                if((intptr_t)server->clients[i].fd == (intptr_t)client_fd->fd) {
                    SSL_shutdown(server->clients[i].ssl);
                    SSL_free(server->clients[i].ssl);
                    server->clients[i].ssl = NULL;
                    close(server->clients[i].fd);
                    server->clients[i].fd = -1;
                }
                if((intptr_t)server->fds[i].fd == (intptr_t)client_fd->fd) {
                    server->fds[i].fd = -1;
                    server->fds[i].events = 0;
                }
            }
            server->client_count--;
            return;
        }
        return;
    }

    buffer[bytes_read] = '\0';
    
    while(bytes_read > 0 && (buffer[bytes_read-1] == '\n' || buffer[bytes_read-1] == '\r')) {
        buffer[--bytes_read] = '\0';
    }
    
    if(bytes_read == 0) return;

    if(strncmp(buffer, "NICK ", 5) == 0 || strncmp(buffer, "/NICK ", 6) == 0) {
        const char *nick_param = strchr(buffer, ' ') + 1;
        handle_nick_command(server, client_fd->fd, nick_param);
    }
    else {
        char time_str[9];
        format_time(time_str, sizeof(time_str));
        
        char formatted_msg[1024];
        snprintf(formatted_msg, sizeof(formatted_msg), "%s - %s: %s\r\n",
            get_client_nickname(server, client_fd->fd), time_str, buffer);
        
        broadcast_message(server, -1, formatted_msg);
    }
}