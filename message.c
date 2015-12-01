//
// Created by nicky on 12/1/15.
//

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "message.h"

#define BUFFER_SIZE 1024

// Отправить сообщение в дескриптор
int sendMessage(int fd, char *string) {
    ssize_t count = write(fd, string, strlen(string));
    if (count == -1) {
        perror("Error: sending message to fd");
        return -1;
    }
    if (count < strlen(string)) {
        fprintf(stderr, "Warning: not whole message was sent\n");
        return -1;
    }
    return 0;
}

// Очистить строку от символов \n
void removeNewLineCharacters(char *string) {
    size_t length = strlen(string);
    for (int i = 1; i < length; i++) {
        if (string[length-i] == '\n')
            string[length-i] = 0;
        else
            break;
    }
}

// Чтение из неблокирующегося дескриптора
ssize_t readNonBlock(int fd, char **buffer, size_t beginSize) {
    size_t size = beginSize;
    if (size < 1)
        size = BUFFER_SIZE;
    if (*buffer == NULL) {
        *buffer = (char *)malloc(size * sizeof(char));
    }
    intmax_t count = 0;
    int legth = 0;
    do {
        count = read(fd, ((*buffer) + legth), BUFFER_SIZE);
        legth += count;
        if (legth == size) {
            size *= 2;
            *buffer = realloc(*buffer, size * sizeof(char));
        }
    } while (count > 0);
    switch(count) {
        case -1: {
            if (errno != EAGAIN) {
                perror("reading non-block error");
                return -1;
            }
            break;
        }
        case 0: {
            fprintf(stderr, "Connection has been closed\n");
            return -1;
        }
        default:
            break;
    }
    removeNewLineCharacters(*buffer);
    size = strlen(*buffer);
    *buffer = (char *)realloc(*buffer, size * sizeof(char));
    return size;
}