#ifndef CORE_PROCESS_PROCESS_H
#define CORE_PROCESS_PROCESS_H

#include <sys/types.h>

#define MAX_PROCESSES 32
#define PROCESS_NAME_LEN 32

typedef enum {
    PROCESS_NEW,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_TERMINATED
} ProcessState;

typedef struct {
    int logical_pid;
    pid_t os_pid;
    char name[PROCESS_NAME_LEN];
    ProcessState state;
    int arrival_time;
    int start_time;
    int end_time;
    int total_work_time;
    int remaining_time;
    int executed_time;
    int waiting_time;
    int io_enabled;
    int io_at;
    int io_wait;
    int io_remaining;
} PCB;

typedef struct {
    PCB entries[MAX_PROCESSES];
    int count;
    int next_pid;
} ProcessTable;

void process_table_init(ProcessTable *table);
int process_table_create(ProcessTable *table,
                         const char *name,
                         int arrival_time,
                         int total_work_time,
                         int io_enabled,
                         int io_at,
                         int io_wait,
                         PCB **pcb_out,
                         int *index_out);
PCB *process_table_get(ProcessTable *table, int index);
const PCB *process_table_get_const(const ProcessTable *table, int index);
int process_table_has_name(const ProcessTable *table, const char *name);
const char *process_state_name(ProcessState state);

#endif
