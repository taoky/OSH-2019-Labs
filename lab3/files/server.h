#ifndef SERVER_H
#define SERVER_H

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "fatal_posix.h"
#include <sys/mman.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#include <sys/epoll.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <linux/limits.h>
#include <errno.h>

// #include <asm-generic/socket.h>

#define SO_REUSEPORT 15  // shut up the complaining arm-linux-gnueabihf-gcc

#define DEBUG(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define EPRINTF(fmt, args...) fprintf(stderr, fmt, ##args)
#define FATAL(msg) perror(msg); exit(-1);

#define BIND_IP_ADDR "0.0.0.0"
#define BIND_PORT 8000
#define MAX_RECV_LEN 8192
#define MAX_SEND_LEN 1048576
#define MAX_PATH_LEN 1024
#define MAX_HOST_LEN 1024
#define MAX_CONN 20

#define MAX_EVENT 1024

#define HTTP_STATUS_200 "200 OK"
#define HTTP_STATUS_404 "404 Not Found"
#define HTTP_STATUS_500 "500 Internal Server Error"

#endif