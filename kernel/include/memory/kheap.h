#ifndef _KERNEL_MEMORY_KHEAP_H
#define _KERNEL_MEMORY_KHEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel.h>

/** Size of the placement memory. */
#define KHEAP_PLACEMENT_SIZE 0x100000

/** Virtual base address of the kernel heap. */
#define KHEAP_HEAP_BASE 0xE0000000

/** Size of the kernel heap. */
#define KHEAP_HEAP_SIZE 0x2000000

#define KHEAP_MAGIC 0xD11EF00D

struct kheap_block {
    uint32_t magic;
    size_t size;
    bool free;
    struct kheap_block* prev;
    struct kheap_block* next;
};

typedef struct kheap_block kheap_block_t;

void kheap_init();

/**
 * Allocate a block of memory with a specified size. The memory
 * is aligned to 4KB.
 * 
 * If the kernel heap is not initialized yet, the memory is allocated
 * from the placement memory. Otherwise, the memory is allocated
 * from the kernel heap.
 * 
 * @param size The size of the block.
 * @return The address of the allocated block.
 */
void* kmalloc_a(uint32_t size);

/**
 * Allocate a block of memory with a specified size.
 * 
 * If the kernel heap is not initialized yet, the memory is allocated
 * from the placement memory. Otherwise, the memory is allocated
 * from the kernel heap.
 * 
 * @param size The size of the block.
 * @return The address of the allocated block.
 */
void* kmalloc(uint32_t size);

/**
 * Free a block of memory allocated by kmalloc.
 * 
 * If the memory block was allocated from the placement memory,
 * it is not freed and the function does nothing.
 * 
 * @param ptr The address of the block to free.
 */
void kfree(void* ptr);

#endif // _KERNEL_MEMORY_KHEAP_H
