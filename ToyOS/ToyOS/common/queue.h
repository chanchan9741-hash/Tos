#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include <stddef.h>

#define QUEUE_CAPACITY 128

typedef struct {
    int items[QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t size;
} IntQueue;

void queue_init(IntQueue *queue);
int queue_is_empty(const IntQueue *queue);
int queue_is_full(const IntQueue *queue);
size_t queue_size(const IntQueue *queue);
int queue_push(IntQueue *queue, int value);
int queue_pop(IntQueue *queue, int *value_out);
int queue_peek(const IntQueue *queue, int *value_out);
int queue_get(const IntQueue *queue, size_t index, int *value_out);

#endif
