//
// Created by nicky on 11/29/15.
//

#ifndef SSHEPOLL_THREADS_H
#define SSHEPOLL_THREADS_H
#include "queue.h"
int createThreads(size_t number, struct Queue *queue);
int destroyThreads();
#endif //SSHEPOLL_THREADS_H
