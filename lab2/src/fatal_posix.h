#ifndef FATAL_H
#define FATAL_H

#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int Open(const char *pathname, int flags);
int Open(const char *pathname, int flags, mode_t mode);
int Close(int fd);
int Dup2(int fildes, int fildes2);
int Pipe(int fildes[2]);
ssize_t Write(int fd, const void *buf, size_t count);
int Mkstemp(char *Template);

#endif