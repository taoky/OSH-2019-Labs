#include "rl.h"

char const *builtin_names[] = {
    "cd", "pwd", "export", "exit", NULL
};

// https://thoughtbot.com/blog/tab-completion-in-gnu-readline
char *builtin_name_generator(const char *text, int state)
{
    static int list_index, len;
    char const *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    if (len == 0) {
        return NULL;
    }

    while ((name = builtin_names[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}

char **builtin_name_completion(const char *text, int start, int end) {
    // rl_attempted_completion_over = 1;
    return rl_completion_matches(text, builtin_name_generator);
}