#include "shell.h"
#include "utils.h"
#include "fatal_posix.h"
#include "rl.h"

string EMPTY_CHAR = " \t\n";

void tsh_prepare(void) {
}

void tsh_leave(void) {
}

char *get_home(void) {
    char *user_home = getenv("HOME");
    if (!user_home) {
        EPRINTF("Warning: Environmental variable 'HOME' not found.");
    }
    return user_home;
}


#define HANDLE_1() if (!this_str.empty()) {output.token_strs.push_back(this_str); \
output.token_types.push_back(this_type); this_str = ""; this_type = TK_CMD;}
#define IS_END(x) (i + x == input_str.size())


void tsh_token_1pass(char input[], tks &output) {
    string input_str = input;

    string this_str = "";
    int this_type = TK_CMD;
    for (size_t i = 0; i != input_str.size(); i++) {
        switch (input_str[i]) {
            case '\\':
                if (IS_END(1)) {
                    // EPRINTF("DEBUG: HIT ENDING \\\n");
                    this_str += '\\';
                    continue;
                } else {
                    // EPRINTF("DEBUG: NOT ENDING \\\n");
                    switch (input_str[i + 1]) {
                        case 'n':
                            this_str += '\n';
                            break;
                        case 'r':
                            this_str += '\r';
                            break;
                        case 't':
                            this_str += '\t';
                            break;
                        default:
                            this_str += input_str[i + 1];
                    }
                    i++;
                }
                break;
            case '|':
                HANDLE_1();
                this_str = "|";
                this_type = TK_PIPE;
                HANDLE_1();
                break;
            case '>':
                if (!is_str_numeric(this_str)) {
                    HANDLE_1();
                }
                this_str += ">";
                if (!IS_END(1)) {
                    if (input_str[i + 1] == '>') {
                        this_str += ">";
                        i++;
                    }
                }
                this_type = TK_RE;
                HANDLE_1();
                break;
            case '<':
                if (!is_str_numeric(this_str)) {
                    HANDLE_1();
                }
                this_str += "<";
                if (!IS_END(1)) {
                    if (!IS_END(2) && input_str[i + 1] == '<' && input_str[i + 2] == '<') {
                        this_str += "<<";
                        i += 2;
                    } else if (input_str[i + 1] == '<') {
                        this_str += "<";
                        i += 1;
                    }
                }
                this_type = TK_RE;
                HANDLE_1();
                break;
            default:
                if (EMPTY_CHAR.find(input_str[i]) != string::npos) {
                    HANDLE_1();
                } else {
                    this_str += input_str[i];
                }
        }
    }

    HANDLE_1();
}

void tsh_token_2pass(tks &args_in) {
    for (size_t i = 0; i < args_in.token_strs.size(); i++) {
        if (args_in.token_types[i] == TK_CMD) {
            if (args_in.token_strs[i][0] == '$') {
                string var = args_in.token_strs[i].substr(1);
                char *res = getenv(var.c_str());
                if (!res) {
                    args_in.token_types[i] = TK_ERROR;
                } else {
                    args_in.token_strs[i] = res;
                }
            }
        }
    }
}

tks tsh_token_process(char input_line[]) {
    tks args;

    tsh_token_1pass(input_line, args);
    tsh_token_2pass(args);

    return args;
}

bool is_tsh_builtins(char *s, char *s_sec) {
    string str = s;
    if (str == "cd" || str == "pwd" || str == "export" || str == "exit") {
        if (str == "export" && !s_sec) {
            strcpy(s, "env");
            return false;
        }
        return true;
    } else {
        return false;
    }
}

int tsh_builtins(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        int chdir_ret = 0;
        char *user_home = NULL;
        if (args[1]) {
            chdir_ret = chdir(args[1]);
        } else { 
            user_home = get_home();
            if (user_home) {
                chdir_ret = chdir(user_home);
            } else {
                // EPRINTF("Missing HOME environmental variable.\n");
                // EPRINTF("If you have manually set that in this shell, exit, \n");
                // EPRINTF("and execute me with HOME again!\n");
            }
        }

        if (chdir_ret) {
            if (args[1]) {
                perror(args[1]);
            } else if (user_home) {
                perror(user_home);
            }
            return -1;
        } else {
            return 0;
        }
    } else if (strcmp(args[0], "pwd") == 0) {
        char *wd = get_current_dir_name();
        if (!wd) {
            perror("get_current_dir_name()");
            return -1;
        } else {
            puts(wd);
            free(wd);
        }
        return 0;
    } else if (strcmp(args[0], "export") == 0) {
        if (!args[1]) {
            return -1;
        } else {
            bool fail = false;
            char *p = strchr(args[1], '=');
            char *name = strndup(args[1], p - args[1]);
            if (!p) {
                EPRINTF("export: Your input does not contain '='.\n");
                fail = true;
            } else {
                *p = '\0';
                int setenv_ret = setenv(name, p + 1, true);
                if (setenv_ret) {
                    perror("export");
                    fail = true;
                }
            }
            free(name);
            if (fail) return -1;
        }
        return 0;
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    return -1;
}

#define NEXT_PIPE (!args.token_types.empty() && args.token_types.front() == TK_PIPE)
#define NEXT_RE (!args.token_types.empty() && args.token_types.front() == TK_RE)
#define NEXT_CMD (!args.token_types.empty() && args.token_types.front() == TK_CMD)
#define POP_FRONT() {args.token_strs.erase(args.token_strs.begin(), args.token_strs.begin() + 1);\
        args.token_types.erase(args.token_types.begin(), args.token_types.begin() + 1);}

#define INPUT_RE() {Close(tmpfile); \
                    tmpfile = open(tmpname, O_RDONLY); \
                    Dup2(tmpfile, target_fd == -1 ? STDIN_FILENO : target_fd); \
                    Close(tmpfile);}

#define AND_SYM_ERR() {EPRINTF("tsh: Not a number after '&'\n");\
                        EPRINTF("tsh: If you would like to redirect to a file descriptor, do not insert space between '&' and the number.\n"); \
                        EPRINTF("tsh: Otherwise, insert a space before '&' (if it is in your filename).\n");}

int last_cmd_status = 0;

void tsh_loop(void)
{
    while (1)
    {
        char *cmd = NULL;
        // int fildes[2];
        int next_in_fd = STDIN_FILENO;
        tks args;

        fflush(stdin);

        rl_attempted_completion_function = builtin_name_completion;
        cmd = readline(((last_cmd_status == 0 ? string("") : (to_string(last_cmd_status & 0377) + "| ")) + 
                        (getuid() ? "$ " : "# ")).c_str());
        if (!cmd) {
            EPRINTF("Got an EOF. Exiting...\n");
            return;
        }
        add_history(cmd);

        args = tsh_token_process(cmd);

        char **sub_args = NULL;

        int fildes[2];

        for (size_t i = 0; i < args.token_strs.size(); i++) {
            if (args.token_types[i] == TK_ERROR) {
                EPRINTF("tsh: cannot find the environmental variable %s\n", args.token_strs[i].c_str());
                goto fail;
            }
        }

        while ((sub_args = next_cmd(args))) {
            if (sub_args[0]) {
                if (!NEXT_PIPE && !NEXT_RE && is_tsh_builtins(sub_args[0], sub_args[1])) {
                    last_cmd_status = tsh_builtins(sub_args);
                } else {  // not builtins or builtins with pipe / file redirect
                    tks reds;
                    bool valid = true;
                    while (NEXT_RE) {
                        reds.token_types.push_back(args.token_types.front());
                        reds.token_strs.push_back(args.token_strs.front());
                        POP_FRONT();
                        if (args.token_strs.empty() || NEXT_RE || NEXT_PIPE) {
                            valid = false;
                            EPRINTF(ERROR_PARSE);
                            break;
                        }
                        reds.token_types.push_back(args.token_types.front());
                        reds.token_strs.push_back(args.token_strs.front());
                        POP_FRONT();
                    }
                    if (NEXT_CMD) {
                        valid = false;
                        EPRINTF(ERROR_PARSE);
                    }
                    if (valid) {
                        if (NEXT_PIPE) {
                            Pipe(fildes);
                        }

                        pid_t pid = fork();
                        pid_t wpid;
                        int wstatus;

                        if (pid == 0) {

                            for (size_t i = 0; i < reds.token_strs.size(); i += 2) {
                                int new_fd = -1;

                                int tmpfile = -1;
                                char tmpname[] = "/tmp/tsh-XXXXXX";

                                size_t find_pos = 0;
                                int target_fd = str_to_number(reds.token_strs[i]);
                                if ((find_pos = reds.token_strs[i].find("<")) != string::npos) {
                                    // handle input
                                    if (find_pos + 3 == reds.token_strs[i].size()) {
                                        // <<<
                                        tmpfile = Mkstemp(tmpname);
                                        Write(tmpfile,
                                            reds.token_strs[i + 1].c_str(),
                                            reds.token_strs[i + 1].size());
                                        INPUT_RE();
                                    } else if (find_pos + 2 == reds.token_strs[i].size()) {
                                        // <<
                                        string stop = reds.token_strs[i + 1];
                                        tmpfile = Mkstemp(tmpname);
                                        while (1) {
                                            char *l = NULL;
                                            size_t len = 0;
                                            int nlen = getline(&l, &len, stdin);
                                            if (stop + "\n" == l) {
                                                break;
                                            }
                                            Write(tmpfile, l, nlen);
                                            free(l);
                                        }
                                        INPUT_RE();
                                    } else {
                                        if (reds.token_strs[i + 1][0] != '&') {
                                            new_fd = Open(reds.token_strs[i + 1].c_str(), O_RDONLY);
                                        } else {
                                            new_fd = str_to_number(reds.token_strs[i + 1].substr(1));
                                            if (new_fd == -1) {
                                                AND_SYM_ERR();
                                                exit(-1);
                                            }
                                        }
                                        Dup2(new_fd, target_fd == -1 ? STDIN_FILENO : target_fd);
                                        Close(new_fd);
                                    }
                                } else if ((find_pos = reds.token_strs[i].find(">")) != string::npos) {
                                    if (reds.token_strs[i + 1][0] != '&') {
                                        new_fd = Open(reds.token_strs[i + 1].c_str(), 
                                                    O_CREAT | O_WRONLY | 
                                                    (find_pos < reds.token_strs[i].size() - 1 &&
                                                        reds.token_strs[i][find_pos + 1] == '>' ? O_APPEND : O_TRUNC),
                                                    0644);
                                    } else {
                                        new_fd = str_to_number(reds.token_strs[i + 1].substr(1));
                                        if (new_fd == -1) {
                                            AND_SYM_ERR();
                                            exit(-1);
                                        }
                                    }
                                    Dup2(new_fd, target_fd == -1 ? STDOUT_FILENO : target_fd);
                                    Close(new_fd);
                                }
                            }


                            if (next_in_fd != STDIN_FILENO) {
                                Dup2(next_in_fd, STDIN_FILENO);
                                Close(next_in_fd);
                            }
                            if (NEXT_PIPE) {
                                Dup2(fildes[1], STDOUT_FILENO);
                                Close(fildes[1]);
                            }

                            if (is_tsh_builtins(sub_args[0], sub_args[1])) {
                                exit(tsh_builtins(sub_args));
                            }
                            execvp(sub_args[0], sub_args);

                            FATAL(sub_args[0]);
                        } else if (pid == -1) {
                            FATAL("fork()");
                        } else {
                            if (next_in_fd != STDIN_FILENO) {
                                Close(next_in_fd);
                                next_in_fd = STDIN_FILENO;
                            }
                            if (NEXT_PIPE) {
                                next_in_fd = fildes[0];
                                Close(fildes[1]);
                                POP_FRONT();
                            }
                            do {
                                wpid = waitpid(pid, &wstatus, WUNTRACED);
                                if (wpid == -1) {
                                    FATAL("waitpid()");
                                }
                            } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
                            if (WIFEXITED(wstatus)) {
                                last_cmd_status = WEXITSTATUS(wstatus);
                            } else {
                                last_cmd_status = -1;
                            }
                        }
                    }
                }
            }
            char **ori = sub_args;
            while (*sub_args) {
                free(*sub_args);
                sub_args++;
            }
            free(ori);
        }

        fail:
            if (!args.token_strs.empty()) {
                EPRINTF(ERROR_PARSE);
            }
        free(cmd);
        // free(args);
    }

}

int main(void)
{
    tsh_prepare();

    tsh_loop();

    tsh_leave();

    return 0;
}