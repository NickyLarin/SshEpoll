//
// Created by nicky on 11/29/15.
//

#include <stdio.h>
#include "event.h"
#include "connection.h"
int handleInEvent(int fd) {
    struct Connection *connection = getConnection(fd);
    if (connection == NULL) {
        fprintf(stderr, "Error: can't find connection in list\n");
        return -1;
    }
    if (checkAuthentication(connection) == 1) {
        printf("Authenticating fd: %d\n", connection->connectionfd);
        int status = authenticate(connection);
        if (status == -1)
            closeConnection(connection);
        if (status == 0)
            return 0;
    }
    return 0;
}

int handleHupEvent(int fd) {
    printf("Here!!");
    struct Connection *connection = getConnection(fd);
    if (closeConnection(connection) == -1)
        return -1;
    return 0;
}