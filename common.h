//
// Created by nicky on 11/30/15.
//

#ifndef SSHEPOLL_COMMON_H
#define SSHEPOLL_COMMON_H
#include <stdio.h>
int setNonBlock(int fd);
size_t fReadString(FILE *file, char **dest);
char **readStringsFromFile(char *path, size_t *resultLength);
void clearStrings(char **strings, size_t length);
#endif //SSHEPOLL_COMMON_H
