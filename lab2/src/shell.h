#ifndef SHELL_H
#define SHELL_H

// #define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
// #include <stdbool.h>


#include <string>
#include <vector>
using namespace std;

enum {
    TK_CMD, TK_PIPE, TK_RE, TK_ERROR
};

typedef struct tks {
    vector <string> token_strs;
    vector <int>    token_types;
} tks;

#define ERROR_PARSE "tsh: Malformed expression\n"

#endif