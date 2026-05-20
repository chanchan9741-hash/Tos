#ifndef CORE_SCHEDULER_SCHEDULER_H
#define CORE_SCHEDULER_SCHEDULER_H

#include "common/queue.h"

#define DEFAULT_RR_QUANTUM 3

typedef enum {
    SCHEDULER_FCFS,
    SCHEDULER_RR,
    SCHEDULER_HRRN
} SchedulerAlgorithm;

typedef struct {
    SchedulerAlgorithm algorithm;
    int is_configured;
    int rr_quantum;
    IntQueue ready_queue;
    IntQueue waiting_queue;
} Scheduler;

void scheduler_init(Scheduler *scheduler);
int scheduler_is_configured(const Scheduler *scheduler);
const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm);
int scheduler_enqueue_ready(Scheduler *scheduler, int process_index);
size_t scheduler_ready_count(const Scheduler *scheduler);
const IntQueue *scheduler_ready_queue(const Scheduler *scheduler);

#endif
