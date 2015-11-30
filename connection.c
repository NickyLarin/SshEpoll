//
// Created by nicky on 11/29/15.
//

#include "connection.h"

#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include "epoll.h"
#include "socket.h"
#include "common.h"

#define START_CONNECTIONS_SIZE 64

static struct Connection *connections;
static size_t connectionsSize;
static size_t connectionsLength;
static pthread_mutex_t mutex;

int initConnections() {
    connectionsSize = START_CONNECTIONS_SIZE;
    connections = (struct Connection *)malloc(connectionsSize * sizeof(struct Connection));
    if (connections == NULL) {
        fprintf(stderr, "Error: allocating memory for connections\n");
    }
    memset(connections, 0, sizeof(connectionsSize * sizeof(struct Connection)));
    connectionsLength = 0;
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
    return 0;
}

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
    if (pthread_mutex_lock(&mutex) != 0) {
        fprintf(stderr, "Error: locking connections mutex\n");
        return -1;
    }
    if (connectionsLength == connectionsSize) {
        size_t oldSize = connectionsSize;
        connectionsSize *= 2;
        struct Connection *tmp = (struct Connection *)realloc(connections, connectionsSize * sizeof(struct Connection));
        if (tmp == NULL) {
            fprintf(stderr, "Error: reallocating memory for connections\n");
            return -1;
        }
        connections = tmp;
        memset(&connections[connectionsLength], 0, (connectionsSize-oldSize)*sizeof(connections[0]));
        printf("Start connections test!\n");
        for (int i = 0; i < connectionsSize; i++) {
            printf("%d\n", connections[i].connectionfd);
        }
    }
    for (int i = 0; i < connectionsSize; i++) {
        if (connections[i].connectionfd == 0) {
            connections[i] = connection;
            connectionsLength++;
            break;
        }
    }
    if (pthread_mutex_unlock(&mutex) != 0) {
        fprintf(stderr, "Error: unlocking connections mutex\n");
        return -1;
    }
    return 0;
}

int acceptNewConnection() {
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);
    int connectionfd = accept(getSocketFd(), &addr, &addrlen);

    if (connectionfd == -1) {
        perror("acception connection error");
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

int destroyConnections() {
    free(connections);
    if (pthread_mutex_destroy(&mutex) != 0) {
        fprintf(stderr, "Error: destroying connections mutex\n");
        return -1;
    }
    return 0;
}