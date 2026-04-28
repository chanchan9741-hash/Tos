#include "user/shell/shell.h"

int main(void) {
    Shell shell;

    shell_init(&shell);
    return shell_run(&shell);
}
