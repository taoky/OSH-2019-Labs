// server.c
// extra ref: CSAPP "Tiny Server" (Ch 11) & Ch 12

#include "server.h"

char cwd[PATH_MAX];
int serv_sock;
int epoll_fd = 0;

bool check_path_security(char *path) {
    if (!path) {
        return false;
    }
    char realp[PATH_MAX];
    realpath(path, realp);
    int i;
    for (i = 0; realp[i] != 0 && cwd[i] != 0; i++) {
        if (realp[i] != cwd[i]) {
            return false;
        }
    }
    if (realp[i] == 0) {
        return false;
    }
    return true;
}

int parse_request(char* request, ssize_t req_len, char** path)
{
    char* req = request;

    ssize_t s1 = 0;
    while(s1 < req_len && req[s1] != ' ') s1++;
    if (s1 == req_len) return -1;

    req[s1] = '.'; *path = req + s1;

    ssize_t s2 = s1 + 1;
    while(s2 < req_len && req[s2] != ' ') s2++;
    if (s2 == req_len) return -1;

    req[s2] = 0;
    return 0;
}

void file_serve(int sock, int file_fd, size_t filesize) {
    off_t ofs = 0;

    while (ofs < filesize) {
        if (sendfile(sock, file_fd, &ofs, filesize - ofs) == -1) {
            if (errno != EAGAIN) {
                perror("sendfile()");
                break;
            }
            continue;
        }
    }
    close(file_fd);
}

void handle_clnt(int clnt_sock)
{
    struct stat statbuf;
    char* response_status = HTTP_STATUS_200;
    size_t content_length = 0;
    bool is_success = false;
    int file_fd;

    char req_buf[MAX_RECV_LEN];

    size_t sfs = 0;
    while (sfs < MAX_RECV_LEN) {
        int ret = read(clnt_sock, req_buf, MAX_RECV_LEN - sfs);
        if (ret == -1) {
            if (errno != EAGAIN) {
                perror("read()");
            }
            break;
        } else if (ret == 0) {
            break;
        } else {
            sfs += ret;
        }
    }

    char* path;
    if (parse_request(req_buf, sfs, &path) == -1) {
        response_status = HTTP_STATUS_500;
    } else {
        if (!check_path_security(path)) {
            response_status = HTTP_STATUS_500;
        } else {
            // DEBUG("%s\n", path);
            file_fd = open(path, O_RDONLY);
            if (file_fd == -1 || fstat(file_fd, &statbuf) == -1) {
                response_status = HTTP_STATUS_404;
            } else {
                if (!(S_ISREG(statbuf.st_mode)) || !(S_IRUSR & statbuf.st_mode)) {
                    response_status = HTTP_STATUS_500;
                    close(file_fd);
                } else {
                    content_length = statbuf.st_size;
                    is_success = true;
                }
            }
        }
    }
    
    char response[256];
    sprintf(response, 
        "HTTP/1.0 %s\r\nContent-Length: %zd\r\n\r\n", 
        response_status, content_length);
    size_t response_len = strlen(response);

    // write(clnt_sock, response, response_len);
    sfs = 0;
    while (sfs < response_len) {
        int ret = write(clnt_sock, response, response_len - sfs);
        if (ret == -1) {
            if (errno != EAGAIN) {
                perror("write()");
            }
            break;
        } else if (ret == 0) {
            break;
        } else {
            sfs += ret;
        }
    }

    if (is_success) {
        file_serve(clnt_sock, file_fd, content_length);
    }

    close(clnt_sock);

}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }
    flags = flags | O_NONBLOCK;

    return (fcntl(fd, F_SETFL, flags) == 0);
}

void handle(int serv_sock) {
    epoll_fd = epoll_create1(0);
    struct epoll_event event, events_list[MAX_EVENT] = {};
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);


    event.events = EPOLLIN | EPOLLET;
    event.data.fd = serv_sock;
    int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_sock, &event);
    if (ret == -1) {
        perror("epoll_ctl()");
    }

    while (1) {
        int ready_fd_cnt = epoll_wait(epoll_fd, events_list, MAX_EVENT, -1);
        for (int i = 0; i < ready_fd_cnt; i++) {
            if (events_list[i].data.fd == serv_sock) {
                while (1) {
                    int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
                    if (clnt_sock == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("accept()");
                        }
                    } else {
                        set_nonblocking(clnt_sock);
                        event.events = EPOLLIN | EPOLLET;
                        event.data.fd = clnt_sock;
                        int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clnt_sock, &event);
                        if (ret == -1) {
                            perror("epoll_ctl");
                        }
                    }
                }
            } else if (events_list[i].events & EPOLLIN) {
                handle_clnt(events_list[i].data.fd);
            } else {
                perror("epoll event");
                close(events_list[i].data.fd);
            }
        }
    }

    Close(epoll_fd);  // Normally unreachable
}

void child_handler() {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        ;
    }

    DEBUG("Process [%d] exited with [%d]\n", pid, status);

    if (fork() == 0) {
        handle(serv_sock);
        exit(0);
    }
}

int set_reuseport(int sock) {
    int enable = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));
}

int set_reuseaddr(int sock) {
    int enable = 1;
    return setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
}

// void suicide() {
//     Close(serv_sock);
//     if (epoll_fd) {
//         Close(epoll_fd);
//     }
//     exit(0);
// }

int main() {
    serv_sock = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int cpu_core_cnt = get_nprocs();

    if (set_reuseport(serv_sock) == -1) {
        perror("SO_REUSEPORT");
    }
    if (set_reuseaddr(serv_sock) == -1) {
        perror("SO_REUSEADDR");
    }
    if (set_nonblocking(serv_sock) == -1) {
        perror("NON-BLOCKING socket failed");
        exit(-1);
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(BIND_IP_ADDR);
    serv_addr.sin_port = htons(BIND_PORT);

    Bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    Listen(serv_sock, SOMAXCONN);

    Signal(SIGCHLD, child_handler);
    Signal(SIGPIPE, SIG_IGN);  // process won't stop when broswer/wget cancels large file downloads
    // Signal(SIGINT, suicide);

    getcwd(cwd, PATH_MAX);

    for (int i = 0; i < cpu_core_cnt * 2; i++) {
        if (fork() == 0) {
            handle(serv_sock);
            return 0;
        }
    }

    for (;;) {
        sleep(255);  // infinity sleep (unless signal SIGCHID received)
    }

    Close(serv_sock);
    return 0;
}