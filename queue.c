#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "queue.h"

#define BEGIN_QUEUE_SIZE 128

extern volatile sig_atomic_t done;

// Инициализируем очередь
int initQueue(struct Queue *queue, size_t sizeOfElement) {
    void **data = (void **)malloc(sizeof(void *) * BEGIN_QUEUE_SIZE);
    if (data == NULL) {
        fprintf(stderr, "Error: initializing queue\n");
        return -1;
    }
    memset(data, 0, sizeof(void *) * BEGIN_QUEUE_SIZE);
    queue->data = data;
    queue->sizeOfElement = sizeOfElement;
    queue->maxSize = BEGIN_QUEUE_SIZE;
    queue->head = 0;
    queue->last = 0;
    pthread_mutexattr_t mutexattr;
    if (pthread_mutexattr_init(&mutexattr) != 0) {
        fprintf(stderr, "Error: initializing queue mutex attributes\n");
        return -1;
    }
    if (pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        fprintf(stderr, "Error: setting type of queue mutex\n");
        return -1;
    }
    if (pthread_mutex_init(&queue->mutex, &mutexattr) != 0) {
        fprintf(stderr, "Error: initializing queue mutex\n");
        return -1;
    }
    if (pthread_mutexattr_destroy(&mutexattr) != 0) {
        fprintf(stderr, "Error: destroying queue mutex attributes\n");
        return -1;
    }
    if (pthread_cond_init(&queue->condition, NULL) != 0) {
        fprintf(stderr, "Error: initializing queue condition variable\n");
        return -1;
    }
    printf("Queue initialized\n");
    return 0;
}

// Блокируем мьютекс
int lockQueue(struct Queue *queue) {
    if (pthread_mutex_lock(&queue->mutex) != 0) {
        fprintf(stderr, "Error: locking queue mutex\n");
        return -1;
    }
    return 0;
}

// Разблокируем мьютекс
int unlockQueue(struct Queue *queue) {
    if (pthread_mutex_unlock(&queue->mutex) != 0) {
        fprintf(stderr, "Error: unlocking queue mutex\n");
        return -1;
    }
    return 0;
}

// Добавляем элемент в очередь
int pushQueue(struct Queue *queue, void *element) {
    if (lockQueue(queue) != 0) {
        return -1;
    }
    if (queue->last == queue->maxSize) {
        queue->maxSize *= 2;
        void **data = (void **)realloc(queue->data, queue->maxSize * sizeof(void *));
        if (data == NULL) {
            fprintf(stderr, "Error: increasing size of queue array\n");
            return -1;
        }
        queue->data = data;
    }
    void *newElement = (void *)malloc(queue->sizeOfElement);
    if (newElement == NULL) {
        fprintf(stderr, "Error: allocating memory for new element in queue\n");
        return -1;
    }
    memcpy(newElement, element, queue->sizeOfElement);
    queue->data[queue->last] = newElement;
    queue->last += 1;
    if (unlockQueue(queue) != 0) {
        return -1;
    }
    return 0;
}

// Сдвигаем элементы в массиве в начало
void moveElementsInQueue(struct Queue *queue) {
    for (int i = 0; i < (queue->last-queue->head); i++) {
        queue->data[i] = queue->data[i+queue->head];
        queue->data[i+queue->head] = NULL;
    }
    queue->last = queue->last-queue->head;
    queue->head = 0;
}

// Достаём элемент из очереди
int popQueue(struct Queue *queue, void *element) {
    if (lockQueue(queue) != 0) {
        return -1;
    }
    if (queue->head == queue->last) {
        fprintf(stderr, "Error: queue is empty\n");
        return -1;
    }
    if (queue->data[queue->head] == NULL) {
        fprintf(stderr, "Error: queue element is null");
        return -1;
    }
    if (element == NULL) {
        fprintf(stderr, "Error: queue get pointer to null");
    }
    memcpy(element, queue->data[queue->head], queue->sizeOfElement);
    free(queue->data[queue->head]);
    queue->head += 1;
    // Если 1/4 в начале очереди пустует, нужно сдвинуть элементы
    if (queue->head >= (queue->maxSize / 4)) {
        moveElementsInQueue(queue);
    }
    if (unlockQueue(queue) != 0) {
        return -1;
    }
    return 0;
}

// Посылаем сигнал
int signalQueue(struct Queue *queue) {
    if (pthread_cond_signal(&queue->condition)) {
        fprintf(stderr, "Error: signaling queue condition variable\n");
        return -1;
    }
    return 0;
}

// Достаём элемент из очереди, встаём в ожидание, если очередь пуста
int popWaitQueue(struct Queue *queue, void *element) {
    if (lockQueue(queue) != 0) {
        return -1;
    }
    while (queue->head == queue->last && !done) {
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 1;
        int status = pthread_cond_timedwait(&queue->condition, &queue->mutex, &timeout);
        if (status == ETIMEDOUT && done) {
            if (unlockQueue(queue) != 0) {
                return -1;
            }
            return 1;
        }
        if (status != 0 && status != ETIMEDOUT) {
            fprintf(stderr, "Error: waiting queue condition variable\n");
            return -1;
        }
    }
    if (queue->head == queue->last) {
        fprintf(stderr, "Error: queue is empty\n");
        return -1;
    }
    if (queue->data[queue->head] == NULL) {
        fprintf(stderr, "Error: queue element is null");
        return -1;
    }
    if (element == NULL) {
        fprintf(stderr, "Error: queue get pointer to null");
    }
    memcpy(element, queue->data[queue->head], queue->sizeOfElement);
    free(queue->data[queue->head]);
    queue->head += 1;
    // Если 1/4 в начале очереди пустует, нужно сдвинуть элементы
    if (queue->head >= (queue->maxSize / 4)) {
        moveElementsInQueue(queue);
    }
    if (unlockQueue(queue) != 0) {
        return -1;
    }
    return 0;
}

// Уничтожить очередь
int destroyQueue(struct Queue *queue) {
    free(queue->data);
    if (pthread_mutex_destroy(&queue->mutex) != 0) {
        fprintf(stderr, "Error: destroying queue mutex\n");
        return -1;
    }
    if (pthread_cond_destroy(&queue->condition) != 0) {
        fprintf(stderr, "Error: destroying queue condition variable\n");
        return -1;
    }
}
