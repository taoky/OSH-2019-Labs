#ifndef RL_H
#define RL_H

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

char *builtin_name_generator(const char *text, int state);
char **builtin_name_completion(const char *text, int start, int end);

#endif