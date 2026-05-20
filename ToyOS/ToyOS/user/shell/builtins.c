#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/util.h"
#include "core/kernel/kernel.h"
#include "core/process/process.h"
#include "user/shell/builtins.h"
#include "user/shell/shell.h"

typedef int (*BuiltinHandler)(Shell *shell, int argc, char *argv[]);

typedef struct {
    const char *name;
    BuiltinHandler handler;
    const char *usage;
    const char *description;
} BuiltinCommand;


static int builtin_set_sched(Shell *shell, int argc, char *argv[]);
static int builtin_run(Shell *shell, int argc, char *argv[]);
static int builtin_cd(Shell *shell, int argc, char *argv[]);
static int builtin_help(Shell *shell, int argc, char *argv[]);
static int builtin_exit(Shell *shell, int argc, char *argv[]);
static int builtin_pwd(Shell *shell, int argc, char *argv[]);
static int builtin_create(Shell *shell, int argc, char *argv[]);
static int builtin_ps(Shell *shell, int argc, char *argv[]);
static void print_create_usage(void);

static const BuiltinCommand builtin_commands[] = {
    { "cd", builtin_cd, "cd <dir>", "Change the current working directory" },
    { "help", builtin_help, "help", "Show available built-in commands" },
    { "exit", builtin_exit, "exit", "Exit the shell" },
    { "pwd", builtin_pwd, "pwd", "Print the current working directory" },
    {
        "create",
        builtin_create,
        "create <name> <total_time>",
        "Create a simulated process PCB and enqueue it in the ready queue"
    },
    {
        "create",
        builtin_create,
        "create <name> <total_time> io <io_at> <io_wait>",
        "Create a simulated process PCB with one optional I/O event"
    },
    { "ps", builtin_ps, "ps", "Show simulated processes and the ready queue" },
    { "set_sched", builtin_set_sched, "set_sched <fcfs | rr> [quantum]", "Set the scheduling algorithm" },
    { "run", builtin_run, "run <ticks>", "Run the scheduler for the given number of ticks" },


};

static const size_t builtin_command_count =
    sizeof(builtin_commands) / sizeof(builtin_commands[0]);

static void print_create_usage(void) {
    fprintf(stderr, "mini-shell: usage:\n");
    fprintf(stderr, "  create <name> <total_time>\n");
    fprintf(stderr, "  create <name> <total_time> io <io_at> <io_wait>\n");
}

static int builtin_cd(Shell *shell, int argc, char *argv[]) {
    (void)shell;

    if (argc < 2) {
        fprintf(stderr, "mini-shell: cd: missing argument\n");
        return BUILTIN_CONTINUE;
    }

    if (argc > 2) {
        fprintf(stderr, "mini-shell: cd: too many arguments\n");
        return BUILTIN_CONTINUE;
    }

    if (chdir(argv[1]) == -1) {
        fprintf(stderr, "mini-shell: cd: %s: %s\n", argv[1], strerror(errno));
    }

    return BUILTIN_CONTINUE;
}

static int builtin_help(Shell *shell, int argc, char *argv[]) {
    size_t i;

    (void)shell;
    (void)argc;
    (void)argv;

    printf("Built-in commands:\n");
    for (i = 0; i < builtin_command_count; ++i) {
        printf("  %-34s %s\n",
               builtin_commands[i].usage,
               builtin_commands[i].description);
    }
    printf("\n");
    printf("External commands are executed with fork() + execvp().\n");

    return BUILTIN_CONTINUE;
}

static int builtin_exit(Shell *shell, int argc, char *argv[]) {
    (void)shell;
    (void)argc;
    (void)argv;
    return BUILTIN_EXIT;
}

static int builtin_pwd(Shell *shell, int argc, char *argv[]) {
    char *cwd;

    (void)shell;
    (void)argv;

    if (argc > 1) {
        fprintf(stderr, "mini-shell: pwd: too many arguments\n");
        return BUILTIN_CONTINUE;
    }

    cwd = util_getcwd_alloc();
    if (cwd == NULL) {
        return BUILTIN_CONTINUE;
    }

    printf("%s\n", cwd);
    free(cwd);
    return BUILTIN_CONTINUE;
}

static int builtin_create(Shell *shell, int argc, char *argv[]) {
    const char *name;
    int total_work_time;
    int io_enabled = 0;
    int io_at = 0;
    int io_wait = 0;
    PCB *pcb;

    if (argc != 3 && argc != 6) {
        print_create_usage();
        return BUILTIN_CONTINUE;
    }

    name = argv[1];
    if (name[0] == '\0') {
        fprintf(stderr, "mini-shell: create: name must not be empty\n");
        return BUILTIN_CONTINUE;
    }

    if (strlen(name) >= PROCESS_NAME_LEN) {
        fprintf(stderr,
                "mini-shell: create: name must be shorter than %d characters\n",
                PROCESS_NAME_LEN);
        return BUILTIN_CONTINUE;
    }

    if (process_table_has_name(&shell->kernel.process_table, name)) {
        fprintf(stderr, "mini-shell: create: process name '%s' already exists\n", name);
        return BUILTIN_CONTINUE;
    }

    if (util_parse_positive_int(argv[2], &total_work_time) != 0) {
        fprintf(stderr,
                "mini-shell: create: total_time must be a positive integer\n");
        return BUILTIN_CONTINUE;
    }

    if (argc == 6) {
        if (strcmp(argv[3], "io") != 0) {
            print_create_usage();
            return BUILTIN_CONTINUE;
        }

        if (util_parse_positive_int(argv[4], &io_at) != 0 ||
            util_parse_positive_int(argv[5], &io_wait) != 0) {
            fprintf(stderr,
                    "mini-shell: create: io_at and io_wait must be positive integers\n");
            return BUILTIN_CONTINUE;
        }

        if (io_at >= total_work_time) {
            fprintf(stderr,
                    "mini-shell: create: io_at must be smaller than total_time\n");
            return BUILTIN_CONTINUE;
        }

        io_enabled = 1;
    }

    if (kernel_create_process(&shell->kernel,
                              name,
                              total_work_time,
                              io_enabled,
                              io_at,
                              io_wait,
                              &pcb) != 0) {
        fprintf(stderr,
                "mini-shell: create: failed to allocate a new process\n");
        return BUILTIN_CONTINUE;
    }

    printf("Created %s (logical PID=%d, state=%s, total=%d)\n",
           pcb->name,
           pcb->logical_pid,
           process_state_name(pcb->state),
           pcb->total_work_time);
    if (pcb->io_enabled) {
        printf("  I/O: enabled at time %d, wait %d\n",
               pcb->io_at,
               pcb->io_wait);
    } else {
        printf("  I/O: disabled\n");
    }

    return BUILTIN_CONTINUE;
}

static int builtin_ps(Shell *shell, int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    kernel_print_process_snapshot(&shell->kernel, stdout);
    return BUILTIN_CONTINUE;
}

int builtins_execute(struct Shell *shell, int argc, char *argv[]) {
    size_t i;

    if (argc == 0) {
        return BUILTIN_CONTINUE;
    }

    for (i = 0; i < builtin_command_count; ++i) {
        if (strcmp(argv[0], builtin_commands[i].name) == 0) {
            return builtin_commands[i].handler(shell, argc, argv);
        }
    }

    return BUILTIN_NOT_FOUND;
}

static int builtin_set_sched(Shell *shell, int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "mini-shell: set_sched: missing algorithm (fcfs or rr)\n");
        return BUILTIN_CONTINUE;
    }

    if (strcmp(argv[1], "fcfs") == 0) {
        shell->kernel.scheduler.algorithm = SCHEDULER_FCFS;
        shell->kernel.scheduler.is_configured = 1;
        printf("Scheduler configured to FCFS.\n");
    } else if (strcmp(argv[1], "rr") == 0) {
        shell->kernel.scheduler.algorithm = SCHEDULER_RR;
        shell->kernel.scheduler.is_configured = 1;
        if (argc >= 3) {
            int quantum;
            if (util_parse_positive_int(argv[2], &quantum) == 0) {
                shell->kernel.scheduler.rr_quantum = quantum;
            }
        }
        printf("Scheduler configured to RR (quantum=%d).\n", shell->kernel.scheduler.rr_quantum);
    } else if (strcmp(argv[1], "hrrn") == 0) {  // <--- 여기서부터 4줄 추가!
        shell->kernel.scheduler.algorithm = SCHEDULER_HRRN;
        shell->kernel.scheduler.is_configured = 1;
        printf("Scheduler configured to HRRN.\n");
    }
    
    
    else {
        fprintf(stderr, "mini-shell: set_sched: unknown algorithm '%s'\n", argv[1]);
    }

    return BUILTIN_CONTINUE;
}

static int builtin_run(Shell *shell, int argc, char *argv[]) {
    int ticks = 1; // 기본 1 tick

    if (!scheduler_is_configured(&shell->kernel.scheduler)) {
        fprintf(stderr, "mini-shell: run: scheduler is not configured. Use 'set_sched' first.\n");
        return BUILTIN_CONTINUE;
    }

    if (argc >= 2) {
        if (util_parse_positive_int(argv[1], &ticks) != 0) {
            fprintf(stderr, "mini-shell: run: invalid ticks value\n");
            return BUILTIN_CONTINUE;
        }
    }

    kernel_run(&shell->kernel, ticks);
    printf("Running scheduler for %d ticks...\n", ticks);
    
    return BUILTIN_CONTINUE;
}