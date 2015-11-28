#include <stdlib.h>

#include "settings.h"
#include "socket.h"

// MAIN
int main(int argc, char *argv[]) {
    // Проверяем количество параметров запуска
    if (argc < 2) {
        fprintf(stderr, "Error: too few parameters\n");
        exit(EXIT_FAILURE);
    }

    // Получаем настройки из файла, путь 1й - параметр запуска
    struct Settings *settings = getSettings(argv[1]);

    // Создаём сокет, порт из настроек
    int socketfd = getSocket(settings->port);

    printf("SocketFd: %d\n", socketfd);
    return 0;
}