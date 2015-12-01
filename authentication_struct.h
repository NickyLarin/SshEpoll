//
// Created by nicky on 12/1/15.
//

#ifndef SSHEPOLL_AUTHENTICATION_STRUCT_H
#define SSHEPOLL_AUTHENTICATION_STRUCT_H
struct Authentication {
    int status;  // 0 - Запрос логина 1 - Проверка логина 2 - Запрос пароля 3 - Проверка пароля 4 - Аутентифицирован
    int attempts;
};
#endif //SSHEPOLL_AUTHENTICATION_STRUCT_H
