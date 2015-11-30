#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdio.h>

#include "signal.h"
#include "settings.h"
#include "socket.h"
#include "queue.h"
#include "threads.h"
#include "epoll.h"
#include "connection.h"

volatile sig_atomic_t done = 0;

void handler(int signum) {
    done = 1;
}

// MAIN
int main(int argc, char *argv[]) {
    // Обработка сигналов
    if (changeSignalHandler(SIGINT, handler) == -1)
        return -1;

    // Проверяем количество параметров запуска
    if (argc < 2) {
        fprintf(stderr, "Error: too few parameters\n");
        exit(EXIT_FAILURE);
    }

    // Получаем настройки из файла, путь - 1й параметр запуска
    struct Settings *settings = getSettings(argv[1]);

    if (createSocket(settings->port) == -1)
        return -1;

    struct Queue queue;
    if (initQueue(&queue, sizeof(struct epoll_event)) == -1)
        return -1;

    if (createThreads(settings->maxThreads, &queue) == -1)
        return -1;

    if (initConnections() == -1)
        return -1;

    if (createEpoll(settings->maxThreads) == -1)
        return -1;

    if (addToEpoll(getSocketFd()) == -1)
        return -1;

    // Main loop
    printf("Starting main loop\n");
    while(!done) {
        waitEvents(&queue);
    }

    // Освобождаем ресурсы
    closeEpoll();
    destroyConnections();
    destroyQueue(&queue);
    closeSocket();
    free(settings);
    return 0;
}