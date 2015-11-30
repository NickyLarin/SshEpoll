//
// Created by nicky on 11/30/15.
//

#ifndef SSHEPOLL_AUTHENTICATION_H
#define SSHEPOLL_AUTHENTICATION_H
struct Authentication {
    int status;  // 0 - Запрос логина 1 - Проверка логина 2 - Запрос пароля 3 - Проверка пароля 4 - Аутентифицирован
    int attempts;
};
struct LoginPassPair {
    char *login;
    char *password;
};
#endif //SSHEPOLL_AUTHENTICATION_H
