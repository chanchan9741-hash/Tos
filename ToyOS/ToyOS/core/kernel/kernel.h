#ifndef CORE_KERNEL_KERNEL_H
#define CORE_KERNEL_KERNEL_H

#include <stdio.h>

#include "core/process/process.h"
#include "core/scheduler/scheduler.h"

typedef struct {
    ProcessTable process_table;
    Scheduler scheduler;
    int current_time;
} Kernel;

void kernel_init(Kernel *kernel);
int kernel_create_process(Kernel *kernel,
                          const char *name,
                          int total_work_time,
                          int io_enabled,
                          int io_at,
                          int io_wait,
                          PCB **pcb_out);
// ToyOS/core/kernel/kernel.h (맨 아래 부분)

void kernel_print_process_snapshot(const Kernel *kernel, FILE *stream);
void kernel_run(Kernel *kernel, int ticks); // <--- 이 줄을 추가하세요!

#endif