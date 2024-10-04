/**
 * @file kheap.h
 * @brief Kernel heap manager.
 * 
 * The kernel heap manager is responsible for managing the kernel heap. The kernel heap is a
 * memory region that is used to allocate memory dynamically. In early stages of the kernel,
 * before the heap manager is initialized, the placement memory, a memory region in the
 * data/bss section, is used to simulate the heap and allocate memory.
 */

#ifndef _KERNEL_MEMORY_KHEAP_H
#define _KERNEL_MEMORY_KHEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory/map.h>

/** Size of the placement memory. */
#define KHEAP_PLACEMENT_SIZE 0x100000

#define KHEAP_MAGIC 0xD11EF00D

struct kheap_block {
    uint32_t magic;
    size_t size;
    bool free;
    struct kheap_block* prev;
    struct kheap_block* next;
};

typedef struct kheap_block kheap_block_t;

/**
 * Initialize the kernel heap.
 */
void kheap_init();

/**
 * Get the total amount of memory in the kernel heap.
 * 
 * @return The total amount of memory.
 */
size_t kheap_get_total_memory_size();

/**
 * Get the amount of free memory in the kernel heap.
 * 
 * @return The amount of free memory.
 */
size_t kheap_get_available_memory_size();

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
void* kmalloc_a(size_t size);

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
void* kmalloc(size_t size);

/**
 * Allocate memory for an array of elements and set the memory to zero.
 * If the kernel heap is not initialized yet, the memory is allocated
 * from the placement memory. Otherwise, the memory is allocated
 * from the kernel heap.
 * 
 * @param num The number of elements.
 * @param size The size of each element.
 * @return The address of the allocated block.
 */
void* kcalloc(size_t num, size_t size);

/**
 * Reallocate a block of memory with a new size.
 * 
 * If the kernel heap is not initialized yet, this operation will
 * fail and the function will return NULL.
 * 
 * @param ptr The address of the block to reallocate.
 * @param size The new size of the block.
 * @return The address of the reallocated block.
 */
void* krealloc(void* ptr, size_t size);

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
