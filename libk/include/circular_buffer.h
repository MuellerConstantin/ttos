#ifndef _LIBK_CIRCULAR_BUFFER_H
#define _LIBK_CIRCULAR_BUFFER_H

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

static inline void circular_buffer_destroy(circular_buffer_t* buffer) {
    kfree(buffer->buffer);
    kfree(buffer);
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

    if(element) {
        void* source = (uint8_t*) buffer->buffer + buffer->tail * buffer->element_size;
        memcpy(element, source, buffer->element_size);
    }

    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->full = false;
}

static inline size_t circular_buffer_size(circular_buffer_t* buffer) {
    if(buffer->full) {
        return buffer->capacity;
    }

    if(buffer->head >= buffer->tail) {
        return buffer->head - buffer->tail;
    }

    return buffer->capacity + buffer->head - buffer->tail;
}

static inline void circular_buffer_get(circular_buffer_t* buffer, size_t index, void* element) {
    if(index >= buffer->capacity) {
        return;
    }

    if(index >= circular_buffer_size(buffer)) {
        return;
    }

    size_t position = (buffer->tail + index) % buffer->capacity;
    void* source = (uint8_t*) buffer->buffer + position * buffer->element_size;
    memcpy(element, source, buffer->element_size);
}

static inline void circular_buffer_set(circular_buffer_t* buffer, size_t index, void* element) {
    if(index >= buffer->capacity) {
        return;
    }

    if(index >= circular_buffer_size(buffer)) {
        return;
    }

    size_t position = (buffer->tail + index) % buffer->capacity;
    void* destination = (uint8_t*) buffer->buffer + position * buffer->element_size;
    memcpy(destination, element, buffer->element_size);
}

static inline bool circular_buffer_empty(circular_buffer_t* buffer) {
    return buffer->tail == buffer->head && !buffer->full;
}

static inline bool circular_buffer_full(circular_buffer_t* buffer) {
    return buffer->full;
}

#endif // _LIBK_CIRCULAR_BUFFER_H
