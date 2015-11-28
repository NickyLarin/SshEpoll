#include <stdlib.h>
#include <signal.h>

#include "settings.h"
#include "socket.h"
#include "signal.h"

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

    // Получаем настройки из файла, путь 1й - параметр запуска
    struct Settings *settings = getSettings(argv[1]);

    // Создаём сокет, порт из настроек
    int socketfd = getSocket(settings->port);

    // Освобождаем ресурсы
    free(settings);
    printf("SocketFd: %d\n", socketfd);
    return 0;
}