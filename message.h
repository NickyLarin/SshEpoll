//
// Created by nicky on 12/1/15.
//

#ifndef SSHEPOLL_MESSAGE_H
#define SSHEPOLL_MESSAGE_H
int sendMessage(int fd, char *string);
ssize_t readNonBlock(int fd, char **buffer, size_t beginSize);
#endif //SSHEPOLL_MESSAGE_H
