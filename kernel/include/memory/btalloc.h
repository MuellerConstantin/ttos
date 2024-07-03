#ifndef _KERNEL_MEMORY_BTALLOC_H
#define _KERNEL_MEMORY_BTALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define BTALLOC_IS_ALIGN(addr) ((((uint32_t) (addr)) | 0xFFFFF000) == 0)
#define BTALLOC_ALIGN(addr) ((((uint32_t) (addr)) & 0xFFFFF000) + 0x1000)

/**
 * Initialize the boot-time memory allocator. This is the first dynamic memory
 * available to the kernel after booting and will be superseded by the kernel's
 * heap allocator once the kernel's paging system is initialized.
 * 
 * @param memory_address The virtual address of the memory to use for allocation.
 * @param size The size of the memory to use for allocation.
 */
void btalloc_init(uint32_t memory_address, size_t size);

/**
 * Allocate a block of memory.
 * 
 * @param size The size of the memory block to allocate.
 * @param align Whether the memory block should be aligned.
 * @return The virtual address of the allocated memory block or NULL if failed.
 */
void* btalloc_malloc(size_t size, bool align);

#endif // _KERNEL_MEMORY_BTALLOC_H
