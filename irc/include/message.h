#ifndef IRC_MESSAGE_H
#define IRC_MESSAGE_H

#include <stdbool.h>

typedef struct {
    char prefix[256];
    char command[32];
    char params[512];
} IRCMessage;

bool parse_message(const char *raw_message, IRCMessage *msg);

void print_message(const IRCMessage *msg);

#endif
