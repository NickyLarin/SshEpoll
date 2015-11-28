#ifndef QUEUE_H
#define QUEUE_H
#include <stdlib.h>
struct Queue {
    void **data;
    int head;
    int last;
    int maxSize;
    size_t sizeOfElement;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
};
int initQueue(struct Queue *queue, size_t sizeOfElement);
int isEmptyQueue(struct Queue *queue);
int pushQueue(struct Queue *queue, void *element);
int popQueue(struct Queue *queue, void *element);
int signalQueue(struct Queue *queue);
int popWaitQueue(struct Queue *queue, void *element);
int destroyQueue(struct Queue *queue);
#endif
