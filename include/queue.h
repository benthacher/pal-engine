#pragma once

#include <stdbool.h>
#include <stdint.h>

struct queue {
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    uint32_t capacity;
    uint8_t *buffer;
};

/**
 * @brief Writes data to queue
 *
 * @param queue
 * @param data
 * @param len
 * @return true if able to write to queue
 * @return false if not
 */
bool queue_write(struct queue *queue, void *data, int len);

/**
 * @brief Reads data from queue
 *
 * @param queue
 * @param data
 * @param len
 * @return true if able to write to queue
 * @return false if not
 */
bool queue_read(struct queue *queue, void *data, int len);

/**
 * @brief Returns the remaining number of bytes in the queue
 *
 * @param queue
 * @return int
 */
int queue_get_remaining(struct queue *queue);

/**
 * @brief Initializes queue with backing buffer
 *
 * @param queue
 * @param buffer buffer to hold queue data
 */
void queue_init(struct queue *queue, uint8_t *buffer, uint32_t capacity);