//
// Created by nicky on 11/29/15.
//

#ifndef SSHEPOLL_CONNECTION_H
#define SSHEPOLL_CONNECTION_H
#include "connection_struct.h"
#include <time.h>
#include "authentication.h"
int initConnections(int connectionTimeout, int timeoutCheckFreq);
int acceptNewConnection();
int destroyConnections();
struct Connection *getConnection(int fd);
int closeConnection(struct Connection *connection);
#endif //SSHEPOLL_CONNECTION_H
