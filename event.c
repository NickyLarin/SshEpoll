//
// Created by nicky on 11/29/15.
//

#include <stdio.h>
#include "event.h"
#include "connection.h"
#include "pty.h"
#include "message.h"

int handleInEvent(int fd) {
    struct Connection *connection = getConnection(fd);
    if (connection == NULL) {
        fprintf(stderr, "Error: can't find connection in list\n");
        return -1;
    }
    if (updateLastEvent(connection) == -1)
        return -1;
    int status = checkAuthentication(connection);
    if (status == -1)
        return -1;
    if (status == 1) {
        printf("Authenticating fd: %d\n", connection->connectionfd);
        int authStatus = authenticate(connection);
        if (authStatus == -1) {
            closeConnection(connection);
            return 0;
        }
        if (authStatus == 1)
            return 0;
    }
    if (connection->ptm == -1) {
        if (createPty(connection) == -1)
            return -1;
        return 0;
    }
    if (fd == connection->connectionfd) {
        if (exchangeMessages(connection->ptm, connection->connectionfd) == -1)
            return -1;
    } else {
        if (exchangeMessages(connection->connectionfd, connection->ptm) == -1)
            return -1;
    }
    return 0;
}

int handleHupEvent(int fd) {
    struct Connection *connection = getConnection(fd);
    if (closeConnection(connection) == -1)
        return -1;
    return 0;
}