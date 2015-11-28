//
// Created by nicky on 11/27/15.
//

#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BEGIN_STRINGS_SIZE 100
#define BUFFER_SIZE 128

int readSettingsFromFile(char *path) {
    FILE *file;
    file = fopen(path, "r");
    if (file == NULL) {
        perror("Error: opening settings file");
        return -1;
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
                return -1;
            }
        }
        strings[length] = string;
        length++;
    }
//    for (int i = 0; i < MAX_STRINGS; i++) {
//        char temp[MAX_STRING_LENGTH];
//        if (fscanf(file, "%s", temp) == EOF) {
//            break;
//        }
//        strings[i] = (char *)malloc(strlen(temp) * sizeof(char));
//        strncpy(strings[i], temp, strlen(temp));
//        count++;
//    }
    fclose(file);
    for (int i = 0; i < length; i++) {
        printf("%s\n", strings[i]);
    }
    return 0;
}

// Читаем строку из файла file в dest
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