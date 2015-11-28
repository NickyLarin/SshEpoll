//
// Created by nicky on 11/29/15.
//

#ifndef SSHEPOLL_THREADS_H
#define SSHEPOLL_THREADS_H
int createThreads(int number, void *(*startFunction)(void *), void *args);
#endif //SSHEPOLL_THREADS_H
