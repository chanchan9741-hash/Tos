#ifndef USER_SHELL_BUILTINS_H
#define USER_SHELL_BUILTINS_H

struct Shell;

enum {
    BUILTIN_NOT_FOUND = -1,
    BUILTIN_CONTINUE = 0,
    BUILTIN_EXIT = 1
};

int builtins_execute(struct Shell *shell, int argc, char *argv[]);

#endif
