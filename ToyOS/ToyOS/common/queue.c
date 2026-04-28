#include "common/queue.h"

void queue_init(IntQueue *queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

int queue_is_empty(const IntQueue *queue) {
    return queue->size == 0;
}

int queue_is_full(const IntQueue *queue) {
    return queue->size >= QUEUE_CAPACITY;
}

size_t queue_size(const IntQueue *queue) {
    return queue->size;
}

int queue_push(IntQueue *queue, int value) {
    if (queue_is_full(queue)) {
        return -1;
    }

    queue->items[queue->tail] = value;
    queue->tail = (queue->tail + 1U) % QUEUE_CAPACITY;
    queue->size++;
    return 0;
}

int queue_pop(IntQueue *queue, int *value_out) {
    if (queue_is_empty(queue)) {
        return -1;
    }

    *value_out = queue->items[queue->head];
    queue->head = (queue->head + 1U) % QUEUE_CAPACITY;
    queue->size--;
    return 0;
}

int queue_peek(const IntQueue *queue, int *value_out) {
    if (queue_is_empty(queue)) {
        return -1;
    }

    *value_out = queue->items[queue->head];
    return 0;
}

int queue_get(const IntQueue *queue, size_t index, int *value_out) {
    size_t actual_index;

    if (index >= queue->size) {
        return -1;
    }

    actual_index = (queue->head + index) % QUEUE_CAPACITY;
    *value_out = queue->items[actual_index];
    return 0;
}
