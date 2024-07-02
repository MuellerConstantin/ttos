#ifndef _KERNEL_MEMORY_PMM_H
#define _KERNEL_MEMORY_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <memory/btalloc.h>

#define PMM_FRAME_SIZE 4096
#define PMM_FRAME_ALIGN 4096
#define PMM_FRAMES_PER_BITMAP_BYTE 8

/**
 * Initialize the Physical Memory Manager.
 * 
 * @param mem_size The total memory size in bytes.
 */
void pmm_init(size_t mem_size);

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
 * Convert a physical address to a frame index.
 * 
 * @param address The physical address.
 * @return The frame index.
 */
uint32_t pmm_address_to_index(void* address);

/**
 * Allocate a block of memory.
 * 
 * @return The address of the allocated frame or NULL if failed.
 */
void* pmm_alloc_frame();

/**
 * Allocate a number of contiguous frames.
 * 
 * @param n The number of frames to allocate.
 * @return The address of the allocated frames or NULL if failed.
 */
void* pmm_alloc_frames(size_t n);

/**
 * Free a block of memory.
 * 
 * @param frame_addr The address of the block to free.
 */
void pmm_free_frame(void *frame_addr);

/**
 * Free a number of contiguous frames.
 * 
 * @param frame_addr The address of the first frame to free.
 * @param n The number of frames to free.
 */
void pmm_free_frames(void *frame_addr, size_t n);

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
