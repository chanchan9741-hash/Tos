#ifndef USER_SHELL_SHELL_H
#define USER_SHELL_SHELL_H

#include "core/kernel/kernel.h"

typedef struct Shell {
    Kernel kernel;
} Shell;

void shell_init(Shell *shell);
int shell_run(Shell *shell);

#endif
