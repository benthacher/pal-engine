#include "queue.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

bool queue_write(struct queue *queue, void *data, int len) {
    assert(data != NULL);

    if (len > queue->capacity - queue->count)
        return false;

    int space_before_wrap = queue->capacity - queue->tail;

    if (len <= space_before_wrap) {
        memcpy(queue->buffer + queue->tail, data, len);
        queue->tail += len;
    } else {
        memcpy(queue->buffer + queue->tail, data, space_before_wrap);
        memcpy(queue->buffer, data + space_before_wrap, len - space_before_wrap);
        queue->tail = len - space_before_wrap;
    }

    queue->count += len;

    return true;
}

bool queue_read(struct queue *queue, void *data, int len) {
    assert(data != NULL);

    if (len > queue->count)
        return false;

    int space_before_wrap = queue->capacity - queue->head;

    if (len <= space_before_wrap) {
        memcpy(data, queue->buffer + queue->head, len);
        queue->head += len;
    } else {
        memcpy(data, queue->buffer + queue->head, space_before_wrap);
        memcpy(data + space_before_wrap, queue->buffer, len - space_before_wrap);
        queue->head = len - space_before_wrap;
    }

    queue->count -= len;

    return true;
}

int queue_get_remaining(struct queue *queue) {
    return queue->count;
}

void queue_init(struct queue *queue, uint8_t *buffer, uint32_t capacity) {
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->capacity = capacity;
    queue->buffer = buffer;
}

