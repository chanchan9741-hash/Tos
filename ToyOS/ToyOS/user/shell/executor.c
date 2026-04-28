#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "user/shell/executor.h"

int executor_run_external(char *argv[]) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 0;
    }

    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "mini-shell: %s: %s\n", argv[0], strerror(errno));
        _exit(EXIT_FAILURE);
    }

    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return 0;
        }
    }

    return 0;
}
