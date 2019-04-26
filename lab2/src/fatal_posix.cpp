#include "fatal_posix.h"

int Open(const char *pathname, int flags) {
    int ret = open(pathname, flags);
    if (ret == -1) {
        FATAL("open()");
    }
    return ret;
}

int Open(const char *pathname, int flags, mode_t mode) {
    int ret = open(pathname, flags, mode);
    if (ret == -1) {
        FATAL("open()");
    }
    return ret;
}

int Close(int fd) {
    int ret = close(fd);
    if (ret == -1) {
        FATAL("close()");
    }
    return ret;
}

int Dup2(int fildes, int fildes2) {
    int ret = dup2(fildes, fildes2);
    if (ret == -1) {
        FATAL("dup2()");
    }
    return ret;
}

// int Dup(int oldfd) {
//     int ret = dup(oldfd);
//     if (ret == -1) {
//         FATAL("dup()");
//     }
//     return ret;
// }

int Pipe(int fildes[2]) {
    int ret = pipe(fildes);
    if (ret == -1) {
        FATAL("pipe()");
    }
    return ret;
}

ssize_t Write(int fd, const void *buf, size_t count) {
    ssize_t ret = write(fd, buf, count);
    if (ret == -1) {
        FATAL("write()");
    }
    return ret;
}

int Mkstemp(char *Template) {
    int ret = mkstemp(Template);
    if (ret == -1) {
        FATAL("mkstemp()");
    }
    return ret;
}