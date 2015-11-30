//
// Created by nicky on 11/29/15.
//

#ifndef SSHEPOLL_CONNECTION_H
#define SSHEPOLL_CONNECTION_H
#include <time.h>
#include "authentication.h"
struct Connection {
    int connectionfd;
    int ptm;
    struct Authentication auth;
    struct LoginPassPair *pair;
    time_t lastEvent;
};
int initConnections();
int acceptNewConnection();
int destroyConnections();
#endif //SSHEPOLL_CONNECTION_H
