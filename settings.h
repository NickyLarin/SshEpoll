//
// Created by nicky on 11/27/15.
//
#ifndef SSHEPOLL_SETTINGS_H
#define SSHEPOLL_SETTINGS_H
struct Settings {
    int maxThreads;
    int port;
    int connectionTimeout;
    int maxPasswordAttempts;
    int timeoutCheckFrequency;
};
struct Settings *getSettings(char *path);
#endif //SSHEPOLL_SETTINGS_H
