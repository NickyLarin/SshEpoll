//
// Created by nicky on 11/29/15.
//

#ifndef SSHEPOLL_EPOLL_H
#define SSHEPOLL_EPOLL_H
#include "queue.h"
int createEpoll(int maxEventsNumber);
int addToEpoll(int fd);
int closeEpoll();
int waitEvents(struct Queue *queue);
#endif //SSHEPOLL_EPOLL_H
