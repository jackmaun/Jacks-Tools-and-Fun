#include "../include/message.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool parse_message(const char *raw_msg, IRCMessage *msg) {
    if(!raw_msg || !msg) {
        return false;
    }
    
    memset(msg, 0, sizeof(IRCMessage));

    const char *cursor = raw_msg;

    if(*cursor == ':') {
        const char *space = strstr(cursor, " ");
        if(!space) {
            strncpy(msg->command, cursor, sizeof(msg->command) - 1);
            return true;
        }
        size_t command_len = space - cursor;
        if(command_len >= sizeof(msg->command)) {
            return false;
        }
        strncpy(msg->command, cursor, command_len);
        cursor = space + 1;

        if(*cursor) {
            strncpy(msg->params, cursor, sizeof(msg->params) - 1);
        }
        return true;
    }
    return false;
}

void print_message(const IRCMessage *msg) {
    if(!msg) return;

    printf("  Prefix: %s\n", msg->prefix[0] ? msg->prefix : "(none)");
    printf("  Command: %s\n", msg->command);
    printf("  Params: %s\n", msg->params[0] ? msg->params : "(none)");
}
