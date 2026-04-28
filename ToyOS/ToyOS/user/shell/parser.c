#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user/shell/parser.h"

#define INITIAL_ARGV_CAPACITY 8

int parser_is_blank_input(const char *line) {
    while (*line != '\0') {
        if (!isspace((unsigned char)*line)) {
            return 0;
        }
        line++;
    }

    return 1;
}

int parser_parse_input(char *line, char ***argv_out) {
    char **argv;
    size_t capacity = INITIAL_ARGV_CAPACITY;
    int argc = 0;
    char *token;

    *argv_out = NULL;

    argv = malloc(capacity * sizeof(*argv));
    if (argv == NULL) {
        perror("malloc");
        return -1;
    }

    token = strtok(line, " \t\r\n");
    while (token != NULL) {
        if ((size_t)(argc + 1) >= capacity) {
            char **new_argv;

            capacity *= 2U;
            new_argv = realloc(argv, capacity * sizeof(*argv));
            if (new_argv == NULL) {
                perror("realloc");
                free(argv);
                return -1;
            }
            argv = new_argv;
        }

        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    argv[argc] = NULL;
    *argv_out = argv;
    return argc;
}

void parser_free_argv(char **argv) {
    free(argv);
}
