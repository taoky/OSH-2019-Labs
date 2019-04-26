#include "utils.h"

bool is_str_numeric(string &s) {
    for (auto i : s) {
        if (!isdigit(i)) {
            return false;
        }
    }
    return true;
}

char **next_cmd(tks &input) {
    if (input.token_strs.empty()) return NULL;
    size_t i;
    char **ret = (char **) malloc(sizeof(char *) * (input.token_strs.size() + 1));
    int ret_p = 0;
    for (i = 0; i < input.token_strs.size() && input.token_types[i] == TK_CMD; i++) {
        ret[ret_p++] = strdup(input.token_strs[i].c_str());
    }
    ret[ret_p] = NULL;
    input.token_strs.erase(input.token_strs.begin(), input.token_strs.begin() + i);
    input.token_types.erase(input.token_types.begin(), input.token_types.begin() + i);
    if (ret_p == 0) {
        free(ret);
        return NULL;
    }
    return ret;
}

int str_to_number(string s) {
    /*  For positive number.
        error: return -1; else return that number */
    // if (!is_str_numeric(s)) return -1;
    try
    {
        int num = stoi(s);
        return num;
    }
    catch(const std::exception& e)
    {
        return -1;
    }
}