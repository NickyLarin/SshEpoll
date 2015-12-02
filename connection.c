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
#include "message.h"
#include "signal.h"

#define START_CONNECTIONS_SIZE 64

extern volatile sig_atomic_t done;

static struct Connection *connections;
static size_t size;
static size_t length;
static pthread_t checkerThread;
static pthread_mutex_t mutex;
static int timeout;
static int freq;

void *checker(void *args);

void emptyHandler(int signum) {
    printf("Checker woke up\n");
};

// Инициализируем список соединений
int initConnections(int connectionTimeout, int timeoutCheckFreq) {
    // Меняем обработчик SIGALRM чтобы разблокировать поток checker
    if (changeSignalHandler(SIGALRM, emptyHandler))
        return -1;
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
    if (pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE) != 0) {
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
    printf("Connections initialized\n");
    freq = timeoutCheckFreq;
    if (pthread_create(&checkerThread, NULL, checker, NULL) != 0) {
        fprintf(stderr, "Error: creating timeout checker thread\n");
        return -1;
    }
    printf("Timeout cheker started\n");
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
            struct Connection empty;
            memset(&empty, 0, sizeof(struct Connection));
            connections[i] = empty;
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
    if (sendMessage(connection->connectionfd, "Connection closed\n") == -1)
        return -1;
    if (close(connection->connectionfd) == -1)
        perror("Error: closing connection");
    if (connection->ptm != -1 && close(connection->ptm))
        perror("Error: closing ptm");
    if (removeConnectionFromList(connection) == -1)
        return -1;
    printf("Connection closed\n");
    return 0;
}

// Обновляем время последнего события
int updateLastEvent(struct Connection *connection) {
    if (connection == NULL) {
        fprintf(stderr, "Error: updating last event\n");
        return -1;
    }
    connection->lastEvent = time(NULL);
    return 0;
}

// Проверяем не наступил ли таймаут соединения
// Возвращаем 0, если таймаут не наступил
// Возвращаем 1, если таймаут наступил
int checkConnectionTimeout(struct Connection *connection) {
    if (difftime(time(NULL), connection->lastEvent) > timeout) {
        return 1;
    }
    return 0;
}

// Функция потока проверки таймаута
void *checker(void *args) {
    while (!done) {
        sleep(freq);
        if (lockConnections() == -1) {
            fprintf(stderr, "Error: locking connections mutex from timeout checker\n");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < length; i++) {
            if (checkConnectionTimeout(&connections[i]) == 1) {
                sendMessage(connections[i].connectionfd, "Connection timeout\n");
                closeConnection(&connections[i]);
            }
        }
        if (unlockConnections() == -1) {
            fprintf(stderr, "Error: unlocking connections mutex from timeout checker\n");
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}

// Освобождаем ресурсы
int destroyConnections() {
    if (pthread_kill(checkerThread, SIGALRM) != 0) {
        fprintf(stderr, "Error: sending SIGUSR1 signal to timeout checker thread\n");
        return -1;
    }
    if (pthread_join(checkerThread, NULL) != 0) {
        fprintf(stderr, "Error: timeout checker thread join\n");
        return -1;
    }
    if (pthread_mutex_destroy(&mutex) != 0) {
        fprintf(stderr, "Error: destroying connections mutex\n");
        return -1;
    }
    free(connections);
    return 0;
}