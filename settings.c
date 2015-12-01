//
// Created by nicky on 11/27/15.
//

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"

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

// Выводим настройки
void printSettings(struct Settings *settings) {
    printf("Settings loaded\n"
                   "Max Threads: %d\n"
                   "Port: %d\n"
                   "Connection Timeout: %d\n"
                   "Max Password Attempts: %d\n"
                   "Timeout Check Frequency: %d\n",
           settings->maxThreads,
           settings->port,
           settings->connectionTimeout,
           settings->maxPasswordAttempts,
           settings->timeoutCheckFrequency);
}

// Получаем настройки из файла
struct Settings *getSettings(char *path) {
    struct Settings *settings = malloc(sizeof(struct Settings));
    memset(settings, 0, sizeof(struct Settings));
    size_t length;
    char **strings = readStringsFromFile(path, &length);
    settings->maxThreads = 1;
    settings->maxThreads = parseString("MaxThreads:", strings, length);
    settings->port = parseString("Port:", strings, length);
    settings->connectionTimeout = parseString("ConnectionTimeout:", strings, length);
    settings->maxPasswordAttempts = parseString("MaxPasswordAttempts:", strings, length);
    settings->timeoutCheckFrequency = parseString("TimeoutCheckFrequency:", strings, length);
    clearStrings(strings, length);
    printSettings(settings);
    return settings;
}