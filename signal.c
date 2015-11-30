//
// Created by nicky on 11/29/15.
//
#include "signal.h"

#include <string.h>
#include <signal.h>
#include <stdio.h>

int changeSignalHandler(int signum, void (*handler)(int)) {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    if (sigaction(signum, &act, 0) == -1) {
        perror("Error: changing signal handler");
        return -1;
    }
    return 0;
}