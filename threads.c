//
// Created by nicky on 11/29/15.
//

#include "threads.h"

#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "event.h"
#include "socket.h"
#include "connection.h"
#include "signal.h"

extern volatile sig_atomic_t done;

static pthread_t *threads;
static size_t length;

// Функция, с которой начинают работу потоки
void *worker(void *args) {
    struct Queue *queue = args;
    while(!done) {
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        if (popWaitQueue(queue, &event) == 1) {
            break;
        }
        if (event.events & EPOLLHUP) {
            printf("Caught hang up event\n");
            handleHupEvent(event.data.fd);
            continue;
        }
        int fd = event.data.fd;
        if (fd == getSocketFd()) {
            printf("Caught new connection event\n");
            fd = acceptNewConnection();
            if (fd == -1)
                continue;
        }
        handleInEvent(fd);
    }
    pthread_exit(NULL);
}

// Создаём потоки
int createThreads(size_t number, struct Queue *queue) {
    threads = (pthread_t *)malloc(number * sizeof(pthread_t));
    length = number;
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
    printf("Threads created\n");
    return 0;
}

// Уничтожаем потоки
int destroyThreads() {
    for (int i = 0; i < length; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error: thread join\n");
            return -1;
        }
    }
    free(threads);
}