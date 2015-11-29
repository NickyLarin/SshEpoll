//
// Created by nicky on 11/29/15.
//

#ifndef SSHEPOLL_EPOLL_H
#define SSHEPOLL_EPOLL_H
int createEpoll();
int addToEpoll(int fd);
int closeEpoll();
#endif //SSHEPOLL_EPOLL_H
