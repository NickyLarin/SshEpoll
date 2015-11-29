//
// Created by nicky on 11/29/15.
//

#include "threads.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/epoll.h>

#include "queue.h"
#include "event.h"
#include "socket.h"
#include "connection.h"

extern volatile sig_atomic_t done;

static pthread_t *threads;

// Функция, с которой начинают работу потоки
void *worker(void *args) {
    struct Queue *queue = args;
    while(!done) {
        struct epoll_event event;
        popWaitQueue(queue, &event);
        if (event.events & EPOLLHUP) {
            handleHupEvent();
        } else if (event.data.fd == getSocketFd()) {
            acceptNewConnection();
        } else {
            handleInEvent();
        }
    }
}

// Создаём потоки
int createThreads(int number, struct Queue *queue) {
    threads = (pthread_t *)malloc(number * sizeof(pthread_t));
    if (threads == NULL) {
        fprintf(stderr, "Error: allocating memory for threads\n");
        return -1;
    }
    for (int i = 0; i < number; i++) {
        if (pthread_create(&threads[i], NULL, worker, (void *)queue) != 0) {
            fprintf(stderr, "Error: creating new thread\n");
            return -1;
        }
    }
    return 0;
}