//
// Created by nicky on 11/29/15.
//

#include <pthread.h>
#include <stdlib.h>
#include "threads.h"

static pthread_mutex_t *threads;

int createThreads(int number) {
    threads = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    return 0;
}

int destroyThreads() {
    return 0;
}
