#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include "shell.h"
using namespace std;

#define DEBUG(fmt, args...) fprintf(stderr, "DEBUG: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#define EPRINTF(fmt, args...) fprintf(stderr, fmt, ##args)
#define FATAL(msg) perror(msg); exit(-1);

bool is_str_numeric(string &s);
char **next_cmd(tks &input);
int str_to_number(string s);

#endif