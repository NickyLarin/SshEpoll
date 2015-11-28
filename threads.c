//
// Created by nicky on 11/29/15.
//

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "threads.h"

static pthread_t *threads;

// Создаём потоки
int createThreads(int number, void *(*startFunction)(void *), void *args) {
    threads = (pthread_t *)malloc(number * sizeof(pthread_t));
    if (threads == NULL) {
        fprintf(stderr, "Error: allocating memory for threads\n");
        return -1;
    }
    for (int i = 0; i < number; i++) {
        if (pthread_create(&threads[i], NULL, startFunction, args) != 0) {
            fprintf(stderr, "Error: creating new thread\n");
            return -1;
        }
    }
    return 0;
}