//
// Created by nicky on 11/29/15.
//

#include "epoll.h"

#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>

static int epollfd;

// Создаём epoll
int createEpoll() {
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("Error: creating epoll");
        return -1;
    }
    return 0;
}

// Добавляем дескриптор в epoll
int addToEpoll(int fd) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = EPOLLET | EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) == -1) {
        perror("Error: adding fd to epoll");
        return -1;
    }
    return 0;
}