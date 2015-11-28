//
// Created by nicky on 11/28/15.
//

#include "socket.h"

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>


// Получение доступных адресов
struct addrinfo *getAvailableAddresses(char *port) {
    struct addrinfo* addresses;

    // Требования к адрессам
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int status = getaddrinfo(NULL, port, &hints, &addresses);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return NULL;
    }
    return addresses;
}

//  Получаем дескриптор сокета привязанный к адресу
int getSocket(int port) {
    char portString[4];
    sprintf(portString, "%d", port);
    struct addrinfo *addresses = getAvailableAddresses(portString);

    int yes = 1;
    int socketfd = 0;

    // Перебор списка подходящих адрессов
    for (struct addrinfo *address = addresses; address != NULL; address = address->ai_next) {
        // Создание неблокирующегося сокета
        socketfd = socket(address->ai_family, address->ai_socktype | SOCK_NONBLOCK, address->ai_protocol);
        if (socketfd == -1) {
            perror("Error: creating socket");
            continue;
        }
        // Установка опции SO_REUSEADDR
        if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            perror("Error: setsockopt");
            continue;
        }
        // Привязка сокета к адресу
        if(bind(socketfd, address->ai_addr, address->ai_addrlen) == -1) {
            close(socketfd);
            perror("Error: binding socket");
            continue;
        }
        // Начинаем слушать сокет
        if (listen(socketfd, SOMAXCONN) == -1) {
            perror("Error: starting to listen socket");
            continue;
        }
        return socketfd;
    }
    return -1;
}