#ifndef _KERNEL_MEMORY_PMM_H
#define _KERNEL_MEMORY_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define PMM_BLOCK_SIZE 4096
#define PMM_BLOCK_ALIGN 4096
#define PMM_BLOCKS_PER_BITMAP_BYTE 8

/**
 * Initialize the Physical Memory Manager.
 * 
 * @param mem_size The total memory size in bytes.
 * @param bitmap_addr The address of the bitmap.
 */
void pmm_init(size_t mem_size, uint32_t bitmap_addr);

/**
 * Unreserve a region of memory, hence marking it as available.
 * 
 * @param base The base address of the region.
 * @param size The size of the region.
 */
void pmm_mark_region_available(uint32_t base, size_t size);

/**
 * Reserve a region of memory, hence marking it as unavailable.
 * 
 * @param base The base address of the region.
 * @param size The size of the region.
 */
void pmm_mark_region_reserved(uint32_t base, size_t size);

/**
 * Allocate a block of memory.
 * 
 * @return The address of the allocated frame.
 */
void* pmm_alloc_frame();

/**
 * Free a block of memory.
 * 
 * @param frame_addr The address of the block to free.
 */
void pmm_free_frame(void *frame_addr);

/**
 * Get the size of the free memory.
 * 
 * @return The size of the free memory in bytes.
 */
size_t pmm_get_free_memory_size();

/**
 * Get the total memory size.
 * 
 * @return The total memory size in bytes.
 */
size_t pmm_get_total_memory_size();

#endif // _KERNEL_MEMORY_PMM_H
