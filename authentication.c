//
// Created by nicky on 11/30/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "authentication.h"
#include "message.h"
#include "common.h"
#include "connection.h"

static struct LoginPassPair *loginData;
static size_t length;
static int maxPasswordAttempts;


// Считываем логины и пароли из файлы
int initAuthentication(char *path, int numberOfAttempts) {
    char **strings = readStringsFromFile(path, &length);
    loginData = (struct LoginPassPair *)malloc(length * sizeof(struct LoginPassPair));
    if (loginData == NULL) {
        fprintf(stderr, "Error: allocating memory for login data\n");
        return -1;
    }
    for (int i = 0; i < length; i++) {
        struct LoginPassPair tmp;
        tmp.login = (char *)malloc(strlen(strings[i]));
        strcpy(tmp.login, strtok(strings[i], ":"));
        tmp.password = (char *)malloc(strlen(strings[i]));
        strcpy(tmp.password, strtok(NULL, ":"));
        if (strlen(tmp.login) > 0 && strlen(tmp.password) > 0)
            loginData[i] = tmp;
    }
    clearStrings(strings, length);
    maxPasswordAttempts = numberOfAttempts;
    return 0;
}

// Проверяем аутентификацию
int checkAuthentication(struct Connection *connection) {
    if (connection == NULL) {
        fprintf(stderr, "Error: Authentication null pointer\n");
        return -1;
    }
    if (connection->auth.status < AUTHENTICATED) {
        return 1;
    }
    return 0;
}

// Возвращаем пару логин-пароль
struct LoginPassPair *getPair(char *login) {
    for (int i = 0; i < length; i++) {
        if (strcmp(login, loginData[i].login) == 0) {
            return &loginData[i];
        }
    }
    return NULL;
}

// Сверяем пароль
int verifyPassword(struct LoginPassPair *pair, char *password) {
    if (strcmp(pair->password, password) != 0) {
        fprintf(stderr, "Login: %s\nWrong password: %s\n", pair->login, password);
        return -1;
    }
    return 0;
}

// Запрашиваем логин
int requestLogin(struct Connection *connection) {
    if (sendMessage(connection->connectionfd, "Enter login: ") == -1) {
        return -1;
    }
    connection->auth.status = LOGIN_CHECK;
    return 0;
}

// Запрашиваем пароль
int requestPassword(struct Connection *connection) {
    if (sendMessage(connection->connectionfd, "Enter password: ") == -1) {
        fprintf(stderr, "Error: sending password msg\n");
        return -1;
    }
    connection->auth.status = PASSWORD_CHECK;
    return 0;
}

// Проверяем логин
int checkLogin(struct Connection *connection) {
    char *login = NULL;
    if (readNonBlock(connection->connectionfd, &login, 0) == -1) {
        fprintf(stderr, "Error: receiving login from connection %d\n", connection->connectionfd);
        return -1;
    }
    connection->pair = getPair(login);
    if (connection->pair == NULL) {
        if (sendMessage(connection->connectionfd, "Wrong login, try again\n") == -1) {
            fprintf(stderr, "Error: sending wrong login msg\n");
            return -1;
        }
        requestLogin(connection);
    } else {
        connection->auth.status = PASSWORD_REQUEST;
        requestPassword(connection);
    }
    free(login);
    return 0;
}

// Проверяем пароль
int checkPassword(struct Connection *connection) {
    char *password = NULL;
    if (readNonBlock(connection->connectionfd, &password, 0) == -1) {
        fprintf(stderr, "Error: receiving password from connection %d\n", connection->connectionfd);
        return -1;
    }
    if (verifyPassword(connection->pair, password) == -1) {
        if (connection->auth.attempts == maxPasswordAttempts) {
            fprintf(stderr, "Many password enter attempts for user: %s\n", connection->pair->login);
            if (sendMessage(connection->connectionfd, "Too many password enter attempts\n") == -1) {
                fprintf(stderr, "Error: sending wrong too many attempts msg\n");
                return -1;
            }
            closeConnection(connection);
            return -1;
        }
        if (sendMessage(connection->connectionfd, "Wrong password, try again\n\n") == -1) {
            fprintf(stderr, "Error: sending wrong password msg\n");
            return -1;
        }
        connection->auth.attempts++;
        requestPassword(connection);
    } else {
        if (sendMessage(connection->connectionfd, "Authentication complete!\n") == -1) {
            fprintf(stderr, "Error: sending password msg\n");
            return -1;
        }
        connection->auth.status = AUTHENTICATED;
    }
    free(password);
    return 0;
}

// Аутентифицируемся
int authenticate(struct Connection *connection) {
    switch (connection->auth.status) {
        case LOGIN_REQUEST:
            requestLogin(connection);
            break;
        case LOGIN_CHECK:
            checkLogin(connection);
            break;
        case PASSWORD_REQUEST:
            requestPassword(connection);
            break;
        case PASSWORD_CHECK:
            checkPassword(connection);
            break;
        default:
            fprintf(stderr, "Error: wrong authentication status of connectionfd\n");
            return -1;
    }
    return 0;
}

// Освобождаем ресурсы
int destroyAuthentication() {
    for (int i = 0; i < length; i++) {
        free(loginData[i].login);
        free(loginData[i].password);
    }
    free(loginData);
    return 0;
}