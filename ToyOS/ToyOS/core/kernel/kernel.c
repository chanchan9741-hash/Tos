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

    // 1. 여기서 p1, p2, p3를 모두 정상적으로 출력합니다.
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
    } // <--- 🌟 괄호가 여기서 안전하게 닫혔습니다!

    // 2. 그 다음 평균 대기 시간을 계산합니다.
    if (kernel->process_table.count > 0) {
        int total_wait = 0;
        for (i = 0; i < kernel->process_table.count; ++i) {
            total_wait += kernel->process_table.entries[i].waiting_time;
        }
        fprintf(stream, "\nAverage Waiting Time: %.2f ticks\n", 
                (double)total_wait / kernel->process_table.count);
    }
}
// ToyOS/core/kernel/kernel.c (맨 아래에 추가)

// ToyOS/core/kernel/kernel.c (맨 아래 함수 수정)

void kernel_run(Kernel *kernel, int ticks) {
    int t;
    for (t = 0; t < ticks; t++) {
        kernel->current_time++; // 전체 시스템 시간 1 증가
        PCB *running_pcb = NULL;
        int i;

        // 1. 현재 RUNNING 상태인 프로세스 찾기
        for (i = 0; i < kernel->process_table.count; i++) {
            if (kernel->process_table.entries[i].state == PROCESS_RUNNING) {
                running_pcb = &kernel->process_table.entries[i];
                break;
            }
        }

        // 2. Ready Queue에서 다음 프로세스 꺼내오기
        if (running_pcb == NULL && queue_size(&kernel->scheduler.ready_queue) > 0) {
            
            // HRRN 알고리즘
            if (kernel->scheduler.algorithm == SCHEDULER_HRRN) {
                int best_pid_index = -1;
                double max_ratio = -1.0;
                size_t q_size = queue_size(&kernel->scheduler.ready_queue);
                
                // 최고 응답 비율 찾기
                for (size_t q_idx = 0; q_idx < q_size; q_idx++) {
                    int pid_index;
                    queue_get(&kernel->scheduler.ready_queue, q_idx, &pid_index);
                    PCB *p = &kernel->process_table.entries[pid_index];
                    
                    double ratio = (double)(p->waiting_time + p->total_work_time) / (double)p->total_work_time;
                    if (ratio > max_ratio) {
                        max_ratio = ratio;
                        best_pid_index = pid_index;
                    }
                }
                
                // 큐 회전시키기 (안전한 방식)
                size_t current_q_size = queue_size(&kernel->scheduler.ready_queue);
                for (size_t k = 0; k < current_q_size; k++) {
                    int pid_index;
                    queue_pop(&kernel->scheduler.ready_queue, &pid_index);
                    
                    if (pid_index != best_pid_index) {
                        queue_push(&kernel->scheduler.ready_queue, pid_index);
                    }
                }
                
                running_pcb = &kernel->process_table.entries[best_pid_index];
            } 
            // FCFS나 RR 알고리즘
            else {
                int next_pid_index;
                queue_pop(&kernel->scheduler.ready_queue, &next_pid_index);
                running_pcb = &kernel->process_table.entries[next_pid_index];
            }

            running_pcb->state = PROCESS_RUNNING;
            if (running_pcb->start_time == -1) {
                running_pcb->start_time = kernel->current_time;
            }
        }

        // 3. 프로세스 실행 및 상태 변경
        if (running_pcb != NULL) {
            running_pcb->remaining_time--;
            running_pcb->executed_time++;

            // 종료
            if (running_pcb->remaining_time <= 0) {
                running_pcb->state = PROCESS_TERMINATED;
                running_pcb->end_time = kernel->current_time;
            }
            // RR 교체
            else if (kernel->scheduler.algorithm == SCHEDULER_RR) {
                if (running_pcb->executed_time % kernel->scheduler.rr_quantum == 0) {
                    running_pcb->state = PROCESS_READY;
                    int pcb_index = running_pcb - kernel->process_table.entries;
                    queue_push(&kernel->scheduler.ready_queue, pcb_index);
                }
            }
        }

        // 4. 대기 시간 증가
        for (size_t q_idx = 0; q_idx < queue_size(&kernel->scheduler.ready_queue); q_idx++) {
            int wait_pid_index;
            if (queue_get(&kernel->scheduler.ready_queue, q_idx, &wait_pid_index) == 0) {
                kernel->process_table.entries[wait_pid_index].waiting_time++;
            }
        }
    }
}