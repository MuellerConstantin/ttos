/**
 * @file pmm.h
 * @brief Definitions for the Physical Memory Manager.
 * 
 * The Physical Memory Manager (PMM) is responsible for managing the physical memory of the system.
 * It keeps track of the available and reserved memory regions and provides functions to allocate and
 * free memory blocks.
 */

#ifndef _KERNEL_MEMORY_PMM_H
#define _KERNEL_MEMORY_PMM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <multiboot.h>
#include <multiboot_util.h>

#define PMM_FRAME_SIZE 4096
#define PMM_FRAME_ALIGN 4096
#define PMM_FRAMES_PER_BITMAP_BYTE 8

#define PMM_MAX_MEMORY_SIZE 0x40000000
#define PMM_MIN_MEMORY_SIZE 0x1F400000

/**
 * Initialize the Physical Memory Manager.
 * 
 * @param multiboot_info The multiboot info structure.
 */
void pmm_init(multiboot_info_t* multiboot_info);

/**
 * Unreserve a region of memory, hence marking it as available.
 * 
 * @param base The phyiscal base address of the region.
 * @param size The size of the region.
 */
void pmm_mark_region_available(void* base, size_t size);

/**
 * Reserve a region of memory, hence marking it as unavailable.
 * 
 * @param base The phyiscal base address of the region.
 * @param size The size of the region.
 */
void pmm_mark_region_reserved(void* base, size_t size);

/**
 * Convert a physical address to a frame index.
 * 
 * @param address The physical address.
 * @return The frame index.
 */
uint32_t pmm_address_to_index(void* address);

/**
 * Convert a frame index to a physical address.
 * 
 * @param index The frame index.
 * @return The physical address.
 */
void* pmm_index_to_address(uint32_t index);

/**
 * Allocate a block of memory.
 * 
 * @return The phyiscal address of the allocated frame or NULL if failed.
 */
void* pmm_alloc_frame();

/**
 * Allocate a number of contiguous frames.
 * 
 * @param n The number of frames to allocate.
 * @return The phyiscal address of the first of the allocated frames or NULL if failed.
 */
void* pmm_alloc_frames(size_t n);

/**
 * Free a block of memory.
 * 
 * @param frame_addr The phyiscal address of the block to free.
 */
void pmm_free_frame(void *frame_addr);

/**
 * Free a number of contiguous frames.
 * 
 * @param frame_addr The phyiscal address of the first of the frames to free.
 * @param n The number of frames to free.
 */
void pmm_free_frames(void *frame_addr, size_t n);

/**
 * Get the size of the phyiscal free memory.
 * 
 * @return The size of the available/free phyiscal memory in bytes.
 */
size_t pmm_get_available_memory_size();

/**
 * Get the total phyiscal memory size.
 * 
 * @return The total phyiscal memory size in bytes.
 */
size_t pmm_get_total_memory_size();

#endif // _KERNEL_MEMORY_PMM_H
