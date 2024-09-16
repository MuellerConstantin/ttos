#ifndef _KERNEL_DS_CIRCULAR_BUFFER_H
#define _KERNEL_DS_CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <memory/kheap.h>

typedef struct circular_buffer circular_buffer_t;

struct circular_buffer {
    void* buffer;
    size_t head;
    size_t tail;
    size_t capacity;
    size_t element_size;
    bool full;
};

static inline circular_buffer_t* circular_buffer_create(size_t capacity, size_t element_size) {
    circular_buffer_t* buffer = (circular_buffer_t*) kmalloc(sizeof(circular_buffer_t));

    if(!buffer) {
        return NULL;
    }

    buffer->buffer = kmalloc(capacity * element_size);

    if(!buffer->buffer) {
        kfree(buffer);
        return NULL;
    }

    buffer->head = 0;
    buffer->tail = 0;
    buffer->capacity = capacity;
    buffer->element_size = element_size;
    buffer->full = false;

    return buffer;
}

static inline void circular_buffer_enqueue(circular_buffer_t* buffer, void* element) {
    void* destination = (uint8_t*) buffer->buffer + buffer->head * buffer->element_size;
    memcpy(destination, element, buffer->element_size);

    if(buffer->full) {
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
    }

    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->full = buffer->head == buffer->tail;
}

static inline void circular_buffer_dequeue(circular_buffer_t* buffer, void* element) {
    if(buffer->tail == buffer->head && !buffer->full) {
        return;
    }

    void* source = (uint8_t*) buffer->buffer + buffer->tail * buffer->element_size;
    memcpy(element, source, buffer->element_size);

    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->full = false;

    return element;
}

static inline bool circular_buffer_empty(circular_buffer_t* buffer) {
    return buffer->tail == buffer->head && !buffer->full;
}

#endif // _KERNEL_DS_CIRCULAR_BUFFER_H
