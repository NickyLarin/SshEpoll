#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "settings.h"
#include "socket.h"
#include "signal.h"
#include "epoll.h"
#include "queue.h"
#include "threads.h"

volatile sig_atomic_t done = 0;

void handler(int signum) {
    done = 1;
}

// MAIN
int main(int argc, char *argv[]) {
    // Обработка сигналов
    changeSignalHandler(SIGINT, handler);

    // Проверяем количество параметров запуска
    if (argc < 2) {
        fprintf(stderr, "Error: too few parameters\n");
        exit(EXIT_FAILURE);
    }

    // Получаем настройки из файла, путь - 1й параметр запуска
    struct Settings *settings = getSettings(argv[1]);

    // Создаём сокет, порт из настроек
    createSocket(settings->port);

    // Создаём очередь
    struct Queue queue;
    initQueue(&queue, sizeof(struct epoll_event));

    // Создаём потоки
    createThreads(settings->maxThreads);

    // Создаём epoll
    createEpoll();

    // Main loop
    printf("Starting main loop\n");
    while(!done) {

    }

    // Освобождаем ресурсы
    closeEpoll();
    destroyQueue(&queue);
    closeSocket();
    free(settings);
    return 0;
}