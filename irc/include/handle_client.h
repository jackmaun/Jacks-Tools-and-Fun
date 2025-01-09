#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "server.h"

void handle_client_message(struct pollfd *client_fd, IRCServer *server);

#endif
