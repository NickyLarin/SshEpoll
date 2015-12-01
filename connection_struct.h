//
// Created by nicky on 12/1/15.
//

#ifndef SSHEPOLL_CONNECTION_STRUCT_H
#define SSHEPOLL_CONNECTION_STRUCT_H
#include "authentication_struct.h"
#include <time.h>
struct Connection {
    int connectionfd;
    int ptm;
    struct Authentication auth;
    struct LoginPassPair *pair;
    time_t lastEvent;
};
#endif //SSHEPOLL_CONNECTION_STRUCT_H
