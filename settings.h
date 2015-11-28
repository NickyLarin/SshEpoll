//
// Created by nicky on 11/27/15.
//
#include <stdio.h>
#ifndef SSHEPOLL_SETTINGS_H
#define SSHEPOLL_SETTINGS_H
int readSettingsFromFile(char *path);
size_t fReadString(FILE *file, char **dest);
#endif //SSHEPOLL_SETTINGS_H
