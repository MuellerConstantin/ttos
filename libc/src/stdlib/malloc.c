#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <hmap.h>

#define HEAP_MAGIC 0xD11EF00D
#define HEAP_PAGE_SIZE 4096

typedef struct heap_block heap_block_t;

struct heap_block {
    uint32_t magic;
    size_t size;
    bool free;
    heap_block_t* prev;
    heap_block_t* next;
};

/**
 * Begin of memory allocated for the kernel heap. This is the first
 * address that can be used for dynamic memory allocation.
 */
static heap_block_t* heap_head = NULL;
static heap_block_t* heap_tail = NULL;

static int32_t heap_increase_size(size_t n_pages);
static inline bool heap_is_valid_address(void* ptr);
static void* heap_find_best_fit(size_t size);

void* malloc(size_t size) {
    // Requests memory from the kernel if the heap is not yet initialized
    if(heap_head == NULL) {
        if(heap_increase_size(1) < 0) {
            return NULL;
        }
    }

    if(size == 0) {
        return NULL;
    }

    size_t total_size = size + sizeof(heap_block_t);

    heap_block_t* best_fit = heap_find_best_fit(total_size);

    if(best_fit == NULL) {
        // Try to increase the heap size
        if(heap_increase_size(1) < 0) {
            return NULL;
        }

        best_fit = heap_find_best_fit(total_size);

        if(best_fit == NULL) {
            return NULL;
        }
    }

    size_t remaining_size = best_fit->size - total_size;

    best_fit->free = false;

    // Split the block if there is enough space for a new block
    if(remaining_size > sizeof(heap_block_t)) {
        heap_block_t* remaining_block = (heap_block_t*) ((uintptr_t) best_fit + total_size);
        remaining_block->free = true;
        remaining_block->magic = HEAP_MAGIC;
        remaining_block->size = remaining_size - sizeof(heap_block_t);
        remaining_block->next = best_fit->next;
        remaining_block->prev = best_fit;

        if(best_fit->next != NULL) {
            best_fit->next->prev = remaining_block;
        } else {
            heap_tail = remaining_block;
        }

        best_fit->size = size;
        best_fit->next = remaining_block;
    }

    return (void*) ((uintptr_t) best_fit + sizeof(heap_block_t));
}

void* calloc(size_t num, size_t size) {
    void* ptr = malloc(num * size);

    if(ptr != NULL) {
        memset(ptr, 0, num * size);
    }

    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if(ptr == NULL) {
        return malloc(size);
    }

    if(size == 0) {
        free(ptr);
        return NULL;
    }

    if(!heap_is_valid_address(ptr)) {
        return NULL;
    }

    heap_block_t* block = (heap_block_t*) ((uintptr_t) ptr - sizeof(heap_block_t));

    if(block->size >= size) {
        return ptr;
    }

    void* new_ptr = malloc(size);

    if(new_ptr == NULL) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);

    free(ptr);

    return new_ptr;
}

void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    if (!heap_is_valid_address(ptr)) {
        return;
    }

    heap_block_t* block = (heap_block_t*) ((uintptr_t) ptr - sizeof(heap_block_t));

    block->free = true;

    // Merge with previous block if it is free
    if (block->prev != NULL && block->prev->magic == HEAP_MAGIC && block->prev->free) {
        block->prev->size += block->size + sizeof(heap_block_t);
        block->prev->next = block->next;

        if (block->next != NULL) {
            block->next->prev = block->prev;
        } else {
            heap_tail = block->prev;
        }

        block = block->prev;

        memset((void*) ((uintptr_t) block + sizeof(heap_block_t)), 0, block->size);
    }

    // Merge with next block if it is free
    if (block->next != NULL && block->next->magic == HEAP_MAGIC && block->next->free) {
        block->size += block->next->size + sizeof(heap_block_t);
        block->next = block->next->next;

        if (block->next != NULL) {
            block->next->prev = block;
        } else {
            heap_tail = block;
        }

        memset((void*) ((uintptr_t) block + sizeof(heap_block_t)), 0, block->size);
    }
}

static inline bool heap_is_valid_address(void* ptr) {
    uintptr_t addr = (uintptr_t) ptr;

    bool in_space = addr >= (uintptr_t) heap_head + sizeof(heap_block_t) && addr <= (uintptr_t) heap_tail + sizeof(heap_block_t);

    if(!in_space) {
        return false;
    }

    heap_block_t* block = (heap_block_t*) ((uintptr_t) ptr - sizeof(heap_block_t));

    return block->magic == HEAP_MAGIC;
}

static int32_t heap_increase_size(size_t n_pages) {
    void* new_heap_limit = hmap_alloc(n_pages);

    if(new_heap_limit == NULL) {
        return -1;
    }

    if(heap_head == NULL) {
        heap_head = (heap_block_t*) ((uint32_t) new_heap_limit - (HEAP_PAGE_SIZE - 1));
        heap_head->magic = HEAP_MAGIC;
        heap_head->size = (HEAP_PAGE_SIZE * n_pages) - sizeof(heap_block_t);
        heap_head->free = true;
        heap_head->next = NULL;
        heap_head->prev = NULL;

        heap_tail = heap_head;
    } else {
        // Check if last block is free, if yes extend it
        if(heap_tail->free) {
            heap_tail->size += (HEAP_PAGE_SIZE * n_pages);
        } else {
            heap_block_t* new_block = (heap_block_t*) ((uint32_t) new_heap_limit - (HEAP_PAGE_SIZE - 1));
            new_block->magic = HEAP_MAGIC;
            new_block->size = HEAP_PAGE_SIZE - sizeof(heap_block_t);
            new_block->free = true;
            new_block->next = NULL;
            new_block->prev = heap_tail;

            heap_tail->next = new_block;
            heap_tail = new_block;
        }
    }

    return 0;
}

static void* heap_find_best_fit(size_t size) {
    heap_block_t* current = heap_head;
    heap_block_t* best_fit = NULL;

    while(current != NULL) {
        if (current->free && current->size >= size) {
            if (best_fit == NULL || current->size < best_fit->size) {
                best_fit = current;
            }
        }

        current = current->next;
    }

    return best_fit;
}
