#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#include "user/shell/builtins.h"
#include "user/shell/executor.h"
#include "user/shell/parser.h"
#include "user/shell/shell.h"

#define PROMPT "mini-shell> "

static void print_prompt(void) {
    printf("%s", PROMPT);
    fflush(stdout);
}

static char *read_input(void) {
    char *line = NULL;
    size_t capacity = 0;
    ssize_t bytes_read;

    bytes_read = getline(&line, &capacity, stdin);
    if (bytes_read == -1) {
        free(line);
        return NULL;
    }

    return line;
}

void shell_init(Shell *shell) {
    kernel_init(&shell->kernel);
}

int shell_run(Shell *shell) {
    while (1) {
        char *line;
        char **argv;
        int argc;
        int builtin_result;

        print_prompt();
        line = read_input();
        if (line == NULL) {
            if (feof(stdin)) {
                printf("\n");
                return EXIT_SUCCESS;
            }

            perror("getline");
            return EXIT_FAILURE;
        }

        if (parser_is_blank_input(line)) {
            free(line);
            continue;
        }

        argc = parser_parse_input(line, &argv);
        if (argc < 0) {
            free(line);
            return EXIT_FAILURE;
        }

        if (argc == 0) {
            parser_free_argv(argv);
            free(line);
            continue;
        }

        builtin_result = builtins_execute(shell, argc, argv);
        if (builtin_result == BUILTIN_EXIT) {
            parser_free_argv(argv);
            free(line);
            return EXIT_SUCCESS;
        }

        if (builtin_result == BUILTIN_NOT_FOUND) {
            executor_run_external(argv);
        }

        parser_free_argv(argv);
        free(line);
    }
}
