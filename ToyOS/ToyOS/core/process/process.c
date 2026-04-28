#include <stdio.h>
#include <string.h>

#include "core/process/process.h"

void process_table_init(ProcessTable *table) {
    memset(table, 0, sizeof(*table));
    table->next_pid = 1;
}

int process_table_create(ProcessTable *table,
                         const char *name,
                         int arrival_time,
                         int total_work_time,
                         int io_enabled,
                         int io_at,
                         int io_wait,
                         PCB **pcb_out,
                         int *index_out) {
    PCB *pcb;

    if (table->count >= MAX_PROCESSES || total_work_time <= 0) {
        return -1;
    }

    pcb = &table->entries[table->count];
    memset(pcb, 0, sizeof(*pcb));

    pcb->logical_pid = table->next_pid++;
    pcb->os_pid = -1;
    snprintf(pcb->name, sizeof(pcb->name), "%s", name);
    pcb->state = PROCESS_NEW;
    pcb->arrival_time = arrival_time;
    pcb->start_time = -1;
    pcb->end_time = -1;
    pcb->total_work_time = total_work_time;
    pcb->remaining_time = total_work_time;
    pcb->executed_time = 0;
    pcb->waiting_time = 0;
    pcb->io_enabled = io_enabled;
    pcb->io_at = io_enabled ? io_at : 0;
    pcb->io_wait = io_enabled ? io_wait : 0;
    pcb->io_remaining = 0;

    if (pcb_out != NULL) {
        *pcb_out = pcb;
    }

    if (index_out != NULL) {
        *index_out = table->count;
    }

    table->count++;
    return 0;
}

PCB *process_table_get(ProcessTable *table, int index) {
    if (index < 0 || index >= table->count) {
        return NULL;
    }

    return &table->entries[index];
}

const PCB *process_table_get_const(const ProcessTable *table, int index) {
    if (index < 0 || index >= table->count) {
        return NULL;
    }

    return &table->entries[index];
}

int process_table_has_name(const ProcessTable *table, const char *name) {
    int i;

    for (i = 0; i < table->count; ++i) {
        if (strcmp(table->entries[i].name, name) == 0) {
            return 1;
        }
    }

    return 0;
}

const char *process_state_name(ProcessState state) {
    switch (state) {
    case PROCESS_NEW:
        return "NEW";
    case PROCESS_READY:
        return "READY";
    case PROCESS_RUNNING:
        return "RUNNING";
    case PROCESS_WAITING:
        return "WAITING";
    case PROCESS_TERMINATED:
        return "TERMINATED";
    default:
        return "UNKNOWN";
    }
}
