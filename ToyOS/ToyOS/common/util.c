#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/util.h"

char *util_getcwd_alloc(void) {
    size_t size = 128;

    while (1) {
        char *buffer = malloc(size);

        if (buffer == NULL) {
            perror("malloc");
            return NULL;
        }

        if (getcwd(buffer, size) != NULL) {
            return buffer;
        }

        free(buffer);

        if (errno != ERANGE) {
            perror("getcwd");
            return NULL;
        }

        if (size > ((size_t)-1) / 2U) {
            fprintf(stderr, "mini-shell: current path is too long\n");
            return NULL;
        }

        size *= 2U;
    }
}

int util_parse_positive_int(const char *text, int *value_out) {
    char *end_ptr;
    long parsed_value;

    errno = 0;
    parsed_value = strtol(text, &end_ptr, 10);
    if (errno != 0 || *text == '\0' || *end_ptr != '\0') {
        return -1;
    }

    if (parsed_value <= 0 || parsed_value > INT_MAX) {
        return -1;
    }

    *value_out = (int)parsed_value;
    return 0;
}
