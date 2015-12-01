//
// Created by nicky on 11/30/15.
//

#include "common.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BEGIN_STRINGS_SIZE 100
#define BUFFER_SIZE 128

// Сделать дескриптор не блокирующимся
int setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error: setting fd to non-block");
        return -1;
    }
    return 0;
}

// Читаем строку из файла
size_t fReadString(FILE *file, char **dest) {
    if (file == NULL) {
        return -1;
    }

    *dest = (char *)malloc(BUFFER_SIZE);  // Do not forget about free()
    memset(*dest, 0, BUFFER_SIZE);

    size_t size = BUFFER_SIZE;
    size_t length = 0;

    char c = (char)getc(file);
    while ((c != '\n') && (c != EOF)) {
        if (length == size) {
            size += BUFFER_SIZE;
            char *tmp = (char *)realloc(*dest, size);
            if (tmp == NULL) {
                fprintf(stderr, "Error: reallocating memory for fReadString\n");
                return -1;
            }
            *dest = tmp;
        }
        (*dest)[length] = c;
        length++;
        c = (char)getc(file);
    }
    return length;
}

// Читаем строки из файла
char **readStringsFromFile(char *path, size_t *resultLength) {
    FILE *file;
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error: opening settings file");
        return NULL;
    }

    size_t length = 0;
    int size = BEGIN_STRINGS_SIZE;
    char **strings = (char **)malloc(size * sizeof(char *));
    memset(strings, 0, size * sizeof(char *));
    char *string;
    while (fReadString(file, &string) > 0) {
        if (length == size) {
            size += BEGIN_STRINGS_SIZE;
            char **tmp = (char **)realloc(strings, size * sizeof(char *));
            if (tmp == NULL) {
                fprintf(stderr, "Error: reallocating memory in readSettingsFromFile\n");
                return NULL;
            }
        }
        strings[length] = string;
        length++;
    }
    fclose(file);
    if (resultLength != NULL) {
        *resultLength = length;
    }
    return strings;
}

// Освобождаем память strings
void clearStrings(char **strings, size_t length) {
    for (int i = 0; i < length; i++) {
        free(strings[i]);
    }
    free(strings);
}