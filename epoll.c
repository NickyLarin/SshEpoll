//
// Created by nicky on 11/29/15.
//

#include "epoll.h"
#include "queue.h"

#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

static int epollfd;
static struct epoll_event *events;
static int eventsSize;

// Создаём epoll
int createEpoll(int maxEventsNumber) {
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("Error: creating epoll");
        return -1;
    }
    events = (struct epoll_event *)malloc(maxEventsNumber * sizeof(struct epoll_event));
    memset(events, 0, maxEventsNumber * sizeof(struct epoll_event));
    if (events == NULL) {
        fprintf(stderr, "Error: allocating memory for epoll events\n");
        return -1;
    }
    eventsSize = maxEventsNumber;
    printf("Epoll created\n");
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
    printf("Descriptor %d added to epoll\n", fd);
    return 0;
}

// Закрываем epoll
int closeEpoll() {
    if (close(epollfd) == -1) {
        perror("Error: closing epoll");
        return -1;
    }
    free(events);
    return 0;
}

// Ожидаем событий epoll и отправляем их в очередь
int waitEvents(struct Queue *queue) {
    int eventsNumber = epoll_wait(epollfd, events, eventsSize, -1);
    if (eventsNumber == -1 && errno == EINTR) {
        return -1;
    } else if (eventsNumber == -1) {
        perror("Error: waiting epoll events");
        return -1;
    }
    if (!eventsNumber) {
        printf("No new events.");
        return 0;
    }
    for (int i = 0; i < eventsNumber; i++) {
        pushQueue(queue, &events[i]);
        signalQueue(queue);
    }
    return 0;
}