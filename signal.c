//
// Created by nicky on 11/29/15.
//

#include "signal.h"

#include <signal.h>
#include <string.h>

int changeSignalHandler(int signum, void (*handler)(int)) {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigaction(signum, &act, 0);
}