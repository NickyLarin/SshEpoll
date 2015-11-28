//
// Created by nicky on 11/27/15.
//

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BEGIN_STRINGS_SIZE 100
#define BUFFER_SIZE 128

// Парсим строку настроек
int parseString(char *string, char **strings, int length) {
    for (int i = 0; i < length; i++) {
        if (strstr(strings[i], string) != NULL) {
            return atoi(strtok(strings[i], string));
        }
    }
    fprintf(stderr, "Error: parsing settings from file");
    exit(EXIT_FAILURE);
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
char **readStringsFromFile(char *path, int *resultLength) {
    FILE *file;
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error: opening settings file");
        return NULL;
    }

    int length = 0;
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
void clearStrings(char **strings, int length) {
    for (int i = 0; i < length; i++) {
        free(strings[i]);
    }
    free(strings);
}

struct Settings *getSettings(char *path) {
    struct Settings *settings = malloc(sizeof(struct Settings));
    memset(settings, 0, sizeof(struct Settings));
    int length;
    char **strings = readStringsFromFile(path, &length);
    settings->maxThreads = 1;
    settings->maxThreads = parseString("MaxThreads:", strings, length);
    settings->port = parseString("Port:", strings, length);
    settings->connectionTimeout = parseString("ConnectionTimeout:", strings, length);
    settings->maxPasswordAttempts = parseString("MaxPasswordAttempts:", strings, length);
    settings->timeoutCheckFrequency = parseString("TimeoutCheckFrequency:", strings, length);
    clearStrings(strings, length);
    return settings;
}