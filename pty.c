//
// Created by nicky on 12/1/15.
//
#define _XOPEN_SOURCE 600
#define _BSD_SOURCE
#include "pty.h"
#include "common.h"
#include "epoll.h"

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

// Создание псевдотерминала
int createPty(struct Connection *connection) {
    int ptm, pts;
    ptm = posix_openpt(O_RDWR);
    if (ptm == -1) {
        perror("creating new plm");
        return -1;
    }
    if (grantpt(ptm) == -1) {
        perror("granting pt access");
        return -1;
    }
    if (unlockpt(ptm) == -1) {
        perror("unlocking pt");
        return -1;
    }
    pts = open(ptsname(ptm), O_RDWR);
    if (pts == -1) {
        perror("opening pts");
        return -1;
    }
    if (setNonBlock(ptm) == -1) {
        fprintf(stderr, "Error: making ptm non-block\n");
        return -1;
    }
    if (setNonBlock(pts) == -1) {
        fprintf(stderr, "Error: making pts non-block\n");
        return -1;
    }

    connection->ptm = ptm;
    if (addToEpoll(ptm) == -1) {
        fprintf(stderr, "Error: adding ptm to epoll\n");
        return -1;
    }
    if (fork()) {
        if (close(pts) == -1) {
            perror("closing pts in parent process");
            return -1;
        }
    } else {
        if (close(ptm) == -1) {
            perror("closing ptm in child process");
            return -1;
        }
        struct termios oldSettings, newSettings;
        if (tcgetattr(pts, &oldSettings) == -1) {
            perror("getting old terminal settings\n");
            return -1;
        }
        newSettings = oldSettings;
        cfmakeraw(&newSettings);
        if (tcsetattr(pts, TCSANOW, &newSettings) == -1) {
            perror("setting new terminal settings\n");
            return -1;
        }
        close(0);
        close(1);
        close(2);

        dup(pts);
        dup(pts);
        dup(pts);

        close(pts);

        setsid();

        ioctl(0, TIOCSCTTY, 1);
        execvp("/bin/bash", NULL);
    }
    return 0;
}