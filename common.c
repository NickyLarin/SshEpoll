//
// Created by nicky on 11/30/15.
//

#include "common.h"

#include <fcntl.h>
#include <stdio.h>

// Сделать дескриптор не блокирующимся
int setNonBlock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error: setting fd to non-block");
        return -1;
    }
    return 0;
}