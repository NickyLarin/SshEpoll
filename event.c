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
        authenticate(connection);
    }
    return 0;
}

int handleHupEvent(int fd) {
    return 0;
}