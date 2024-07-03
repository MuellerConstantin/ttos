#ifndef _KERNEL_MEMORY_KHEAP_H
#define _KERNEL_MEMORY_KHEAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <kernel.h>

#define KHEAP_PLACEMENT_MEMORY_SIZE 0x100000

#define KHEAP_IS_ALIGNED(addr) ((((uint32_t) (addr)) | 0xFFFFF000) == 0)
#define KHEAP_ALIGN(addr) ((((uint32_t) (addr)) & 0xFFFFF000) + 0x1000)

/**
 * Allocate a block of memory with a specified size. The memory
 * is aligned to 4KB.
 * 
 * @param size The size of the block.
 * @return The address of the allocated block.
 */
void* kmalloc_a(uint32_t size);

/**
 * Allocate a block of memory with a specified size.
 * 
 * @param size The size of the block.
 * @return The address of the allocated block.
 */
void* kmalloc(uint32_t size);

#endif // _KERNEL_MEMORY_KHEAP_H
