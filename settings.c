//
// Created by nicky on 11/27/15.
//

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_STRING_LENGTH 1024
#define MAX_STRINGS 100

int readSettingsFromFile(char *path) {
    FILE *file;
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error: opening settings file");
        return -1;
    }
    char *strings[MAX_STRINGS];
    int count = 0;
    for (int i = 0; i < MAX_STRINGS; i++) {
        char temp[MAX_STRING_LENGTH];
        if (fscanf(file, "%s", temp) == EOF) {
            break;
        }
        strings[i] = (char *)malloc(strlen(temp) * sizeof(char));
        strncpy(strings[i], temp, strlen(temp));
        count++;
    }
    fclose(file);
    for (int i = 0; i < count; i++) {
        printf("%s\n", strings[i]);
    }
    return 0;
}