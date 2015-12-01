//
// Created by nicky on 11/30/15.
//

#ifndef SSHEPOLL_AUTHENTICATION_H
#define SSHEPOLL_AUTHENTICATION_H
#define LOGIN_REQUEST 0
#define LOGIN_CHECK 1
#define PASSWORD_REQUEST 2
#define PASSWORD_CHECK 3
#define AUTHENTICATED 4
#include "connection_struct.h"
struct LoginPassPair {
    char *login;
    char *password;
};
int initAuthentication(char *path, int numberOfAttempts);
int checkAuthentication(struct Connection *connection);
int authenticate(struct Connection *connection);
int destroyAuthentication();
#endif //SSHEPOLL_AUTHENTICATION_H
