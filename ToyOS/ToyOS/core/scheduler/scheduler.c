#include "core/scheduler/scheduler.h"

void scheduler_init(Scheduler *scheduler) {
    scheduler->algorithm = SCHEDULER_FCFS;
    scheduler->is_configured = 0;
    scheduler->rr_quantum = DEFAULT_RR_QUANTUM;
    queue_init(&scheduler->ready_queue);
    queue_init(&scheduler->waiting_queue);
}

int scheduler_is_configured(const Scheduler *scheduler) {
    return scheduler->is_configured;
}

const char *scheduler_algorithm_name(SchedulerAlgorithm algorithm) {
    switch (algorithm) {
    case SCHEDULER_FCFS:
        return "FCFS";
    case SCHEDULER_RR:
        return "RR";

    case SCHEDULER_HRRN:    // <--- 추가
        return "HRRN";
    default:
        return "UNKNOWN";
    }
}

int scheduler_enqueue_ready(Scheduler *scheduler, int process_index) {
    return queue_push(&scheduler->ready_queue, process_index);
}

size_t scheduler_ready_count(const Scheduler *scheduler) {
    return queue_size(&scheduler->ready_queue);
}

const IntQueue *scheduler_ready_queue(const Scheduler *scheduler) {
    return &scheduler->ready_queue;
}
