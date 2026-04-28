#include <stdio.h>

#include "core/kernel/kernel.h"

static void kernel_print_named_queue(const Kernel *kernel,
                                     FILE *stream,
                                     const char *label,
                                     const IntQueue *queue) {
    size_t queue_index;

    fprintf(stream, "%s: ", label);
    if (queue_is_empty(queue)) {
        fprintf(stream, "(empty)\n");
        return;
    }

    for (queue_index = 0; queue_index < queue_size(queue); ++queue_index) {
        int process_index;
        const PCB *pcb;

        if (queue_get(queue, queue_index, &process_index) != 0) {
            continue;
        }

        pcb = process_table_get_const(&kernel->process_table, process_index);
        if (pcb == NULL) {
            continue;
        }

        fprintf(stream, "%s", pcb->name);
        if (queue_index + 1U < queue_size(queue)) {
            fprintf(stream, " ");
        }
    }

    fprintf(stream, "\n");
}

static void kernel_print_running_process(const Kernel *kernel, FILE *stream) {
    int i;

    fprintf(stream, "Running: ");
    for (i = 0; i < kernel->process_table.count; ++i) {
        const PCB *pcb = &kernel->process_table.entries[i];

        if (pcb->state == PROCESS_RUNNING) {
            fprintf(stream, "%s\n", pcb->name);
            return;
        }
    }

    fprintf(stream, "(none)\n");
}

static void kernel_print_processes_in_state(const Kernel *kernel,
                                            FILE *stream,
                                            const char *label,
                                            ProcessState state) {
    int i;
    int found = 0;

    fprintf(stream, "%s: ", label);
    for (i = 0; i < kernel->process_table.count; ++i) {
        const PCB *pcb = &kernel->process_table.entries[i];

        if (pcb->state != state) {
            continue;
        }

        if (found) {
            fprintf(stream, " ");
        }
        fprintf(stream, "%s", pcb->name);
        found = 1;
    }

    if (!found) {
        fprintf(stream, "(empty)");
    }

    fprintf(stream, "\n");
}

void kernel_init(Kernel *kernel) {
    process_table_init(&kernel->process_table);
    scheduler_init(&kernel->scheduler);
    kernel->current_time = 0;
}

int kernel_create_process(Kernel *kernel,
                          const char *name,
                          int total_work_time,
                          int io_enabled,
                          int io_at,
                          int io_wait,
                          PCB **pcb_out) {
    int process_index;
    PCB *pcb;

    if (process_table_create(&kernel->process_table,
                             name,
                             kernel->current_time,
                             total_work_time,
                             io_enabled,
                             io_at,
                             io_wait,
                             &pcb,
                             &process_index) != 0) {
        return -1;
    }

    pcb->state = PROCESS_READY;

    if (scheduler_enqueue_ready(&kernel->scheduler, process_index) != 0) {
        pcb->state = PROCESS_NEW;
        return -1;
    }

    if (pcb_out != NULL) {
        *pcb_out = pcb;
    }

    return 0;
}

void kernel_print_process_snapshot(const Kernel *kernel, FILE *stream) {
    int i;

    if (scheduler_is_configured(&kernel->scheduler)) {
        fprintf(stream, "Scheduler: %s",
                scheduler_algorithm_name(kernel->scheduler.algorithm));
        if (kernel->scheduler.algorithm == SCHEDULER_RR) {
            fprintf(stream, " (quantum=%d)", kernel->scheduler.rr_quantum);
        }
        fprintf(stream, "\n");
    }

    kernel_print_running_process(kernel, stream);
    kernel_print_named_queue(kernel,
                             stream,
                             "Ready Queue",
                             scheduler_ready_queue(&kernel->scheduler));
    kernel_print_named_queue(kernel,
                             stream,
                             "Waiting Queue",
                             &kernel->scheduler.waiting_queue);
    kernel_print_processes_in_state(kernel,
                                    stream,
                                    "Terminated",
                                    PROCESS_TERMINATED);
    fprintf(stream, "\n");

    if (kernel->process_table.count == 0) {
        fprintf(stream, "No simulated processes have been created.\n");
        return;
    }

    fprintf(stream,
            "%-4s %-8s %-11s %-7s %-7s %-7s %-7s %-3s %-5s %-5s\n",
            "PID",
            "Name",
            "State",
            "Arrival",
            "Total",
            "Remain",
            "Wait",
            "IO",
            "IO@",
            "IOW");

    for (i = 0; i < kernel->process_table.count; ++i) {
        const PCB *pcb = &kernel->process_table.entries[i];
        fprintf(stream,
                "%-4d %-8s %-11s %-7d %-7d %-7d %-7d %-3s %-5d %-5d\n",
                pcb->logical_pid,
                pcb->name,
                process_state_name(pcb->state),
                pcb->arrival_time,
                pcb->total_work_time,
                pcb->remaining_time,
                pcb->waiting_time,
                pcb->io_enabled ? "Y" : "N",
                pcb->io_enabled ? pcb->io_at : -1,
                pcb->io_enabled ? pcb->io_wait : -1);
    }
}
