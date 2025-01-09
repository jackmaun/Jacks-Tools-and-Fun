#include "../include/server.h"
#include <winsock2.h>
#include <windows.h>

int poll(struct pollfd *fds, unsigned int nfds, int timeout) {
    fd_set read_fds, write_fds, error_fds;
    struct timeval tv, *ptv;
    int max_fd = -1;
    int ready;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    for (unsigned int i = 0; i < nfds; i++) {
        if (fds[i].fd < 0) continue;
        
        if (fds[i].events & POLLIN)
            FD_SET(fds[i].fd, &read_fds);
        if (fds[i].events & POLLOUT)
            FD_SET(fds[i].fd, &write_fds);
        
        max_fd = (fds[i].fd > max_fd) ? fds[i].fd : max_fd;
    }

    if (timeout < 0) {
        ptv = NULL;
    } else {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        ptv = &tv;
    }

    ready = select(max_fd + 1, &read_fds, &write_fds, NULL, ptv);

    if (ready > 0) {
        for (unsigned int i = 0; i < nfds; i++) {
            fds[i].revents = 0;
            if (FD_ISSET(fds[i].fd, &read_fds))
                fds[i].revents |= POLLIN;
            if (FD_ISSET(fds[i].fd, &write_fds))
                fds[i].revents |= POLLOUT;
        }
    }

    return ready;
}

#ifndef POLLIN
#define POLLIN      0x0001
#define POLLOUT     0x0004
#define POLLERR     0x0008
#define POLLHUP     0x0010
#define POLLNVAL    0x0020
#endif