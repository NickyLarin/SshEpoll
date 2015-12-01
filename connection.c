//
// Created by nicky on 11/29/15.
//

#include "connection.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "epoll.h"
#include "socket.h"
#include "common.h"

#define START_CONNECTIONS_SIZE 64

extern volatile sig_atomic_t done;

static struct Connection *connections;
static size_t size;
static size_t length;
static pthread_mutex_t mutex;
static int timeout;

void *checker(void *args);

// Инициализируем список соединений
int initConnections(int connectionTimeout, int timeoutCheckFreq) {
    size = START_CONNECTIONS_SIZE;
    connections = (struct Connection *)malloc(size * sizeof(struct Connection));
    if (connections == NULL) {
        fprintf(stderr, "Error: allocating memory for connections\n");
    }
    memset(connections, 0, sizeof(size * sizeof(struct Connection)));
    length = 0;
    pthread_mutexattr_t mutexattr;
    if (pthread_mutexattr_init(&mutexattr) != 0) {
        fprintf(stderr, "Error: initializing connections mutex attributes\n");
        return -1;
    }
    if (pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        fprintf(stderr, "Error: setting connections mutex type\n");
        return -1;
    }
    if (pthread_mutex_init(&mutex, &mutexattr) != 0) {
        fprintf(stderr, "Error: initializing connections mutex\n");
        return -1;
    }
    if (pthread_mutexattr_destroy(&mutexattr) != 0) {
        fprintf(stderr, "Error: destroying mutex attributes\n");
        return -1;
    }
    timeout = connectionTimeout;
    pthread_t checkerThread;
    int freq = timeoutCheckFreq;
    if (pthread_create(&checkerThread, NULL, checker, (void *)freq) != 0) {
        fprintf(stderr, "Error: creating timeout checker thread\n");
        return -1;
    }
    return 0;
}

// Блокируем мьютекс
int lockConnections() {
    if (pthread_mutex_lock(&mutex) != 0) {
        fprintf(stderr, "Error: locking connections mutex\n");
        return -1;
    }
    return 0;
}

// Разблокируем мьютекс
int unlockConnections() {
    if (pthread_mutex_unlock(&mutex) != 0) {
        fprintf(stderr, "Error: unlocking connections mutex\n");
        return -1;
    }
    return 0;
}

// Добавляем новое соединение в список
int addConnectionToList(int connectionfd) {
    struct Connection connection;
    memset(&connection, 0, sizeof(connection));
    connection.connectionfd = connectionfd;
    connection.ptm = -1;
    struct Authentication auth;
    memset(&auth, 0, sizeof(auth));
    connection.auth = auth;
    connection.lastEvent = time(NULL);
    connection.pair = NULL;
    if (lockConnections() == -1)
        return -1;
    if (length == size) {
        size_t oldSize = size;
        size *= 2;
        struct Connection *tmp = (struct Connection *)realloc(connections, size * sizeof(struct Connection));
        if (tmp == NULL) {
            fprintf(stderr, "Error: reallocating memory for connections\n");
            return -1;
        }
        connections = tmp;
        memset(&connections[length], 0, (size - oldSize) * sizeof(connections[0]));
        printf("Start connections test!\n");
        for (int i = 0; i < size; i++) {
            printf("%d\n", connections[i].connectionfd);
        }
    }
    for (int i = 0; i < size; i++) {
        if (connections[i].connectionfd == 0) {
            connections[i] = connection;
            length++;
            break;
        }
    }
    if (unlockConnections() == -1)
        return -1;
    return 0;
}

// Удаляем соединение из списка
int removeConnectionFromList(struct Connection *connection) {
    if (lockConnections() == -1)
        return -1;
    for (int i = 0; i < length; i++) {
        if (&connections[i] == connection) {
            memset(&connections[i], 0, sizeof(connections[i]));
            length--;
            break;
        }
    }
    if (unlockConnections() == -1)
        return -1;
    return 0;
}

// Принимаем новое соеднинение из сокета
int acceptNewConnection() {
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    int connectionfd = accept(getSocketFd(), &addr, &addrlen);

    if (connectionfd == -1) {
        perror("Error: acception connection");
        return -1;
    }
    if (setNonBlock(connectionfd) == -1)
        return -1;
    if (addToEpoll(connectionfd) == -1)
        return -1;
    if (addConnectionToList(connectionfd) == -1)
        return -1;
    printf("New connection accepted\n");
    return connectionfd;
}

// Освобождаем ресурсы
int destroyConnections() {
    free(connections);
    if (pthread_mutex_destroy(&mutex) != 0) {
        fprintf(stderr, "Error: destroying connections mutex\n");
        return -1;
    }
    return 0;
}

// Возвращаем указатель на соединение из списка
struct Connection *getConnection(int fd) {
    struct Connection *result = NULL;
    if (lockConnections() == -1)
        return NULL;
    for (int i = 0; i < length; i++) {
        if (connections[i].connectionfd == fd || connections[i].ptm == fd) {
            result = &connections[i];
            break;
        }
    }
    if (unlockConnections() == -1)
        return NULL;
    return result;
}

// Закрываем соединение
int closeConnection(struct Connection *connection) {
    if (close(connection->connectionfd) == -1) {
        perror("Error: closing connection");
    }
    if (close(connection->ptm) == -1) {
        perror("Error: closing ptm");
    }
    if (removeConnectionFromList(connection) == -1)
        return -1;
    return 0;
}

// Проверяем не наступил ли таймаут соединения
// Обновляем если не наступил
// Возвращаем 0, если таймаут не наступил и был обновлен
// Возвращаем 1, если таймаут наступил
int checkConnectionTimeout(struct Connection *connection) {
    if (difftime(time(NULL), connection->lastEvent) > timeout) {
        fprintf(stderr, "Connection %d closed on timeout\n", connection->connectionfd);
        if (closeConnection(connection) == -1) {
            fprintf(stderr, "Error: closing connection on timeout\n");
            return -1;
        }
        return 1;
    } else {
        connection->lastEvent = time(NULL);
        return 0;
    }
}

// Функция потока проверки таймаута
void *checker(void *args) {
    int freq = (int)args;
    while (!done) {
        sleep(freq);
        if (lockConnections() == -1)
            return NULL;
        for (int i = 0; i < length; i++) {
            checkConnectionTimeout(&connections[i]);
        }
        if (unlockConnections() == -1)
            return NULL;
    }
}