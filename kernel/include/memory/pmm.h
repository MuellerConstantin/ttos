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
#include <util/string.h>
#include <stdbool.h>
#include <util/numeric.h>
#include <multiboot.h>
#include <util/linked_list.h>

#define PMM_PAS_SIZE 0xFFFFFFFF

#define PMM_FRAME_SIZE 4096
#define PMM_FRAMES_PER_BITMAP_BYTE 8

typedef struct pmm_memory_region pmm_memory_region_t;

struct pmm_memory_region {
    uint32_t base;
    uint32_t length;
    uint32_t type;
};

/**
 * Initialize the Physical Memory Manager.
 * 
 * @param multiboot_info The multiboot info structure.
 */
void pmm_init(multiboot_info_t* multiboot_info);

/**
 * Get the memory regions detected by the BIOS.
 * 
 * @return The memory regions detected by the BIOS.
 */
const linked_list_t* pmm_get_memory_regions();

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
 * Mark a frame as reserved.
 * 
 * @param frame_addr The physical address of the frame to mark as reserved.
 */
void pmm_mark_frame_reserved(void* frame_addr);

/**
 * Mark a region as available.
 * 
 * @param frame_addr The physical address of the first frame in the region.
 */
void pmm_mark_frame_available(void* frame_addr);

/**
 * Allocate a single frame.
 * 
 * @return The phyiscal address of the allocated frame or NULL if failed.
 */
void* pmm_alloc_frame();

/**
 * Allocate a number of contiguous frames.
 * 
 * @param n The number of frames to allocate.
 * @return The phyiscal address of the allocated frames or NULL if failed.
 */
void* pmm_alloc_frames(size_t n);

/**
 * Free a single frame.
 * 
 * @param address The phyiscal address of the frame to free.
 */
void pmm_free_frame(void* address);

/**
 * Free a number of contiguous frames.
 * 
 * @param frame_addr The phyiscal address of the first frame to free.
 * @param n The number of frames to free.
 */
void pmm_free_frames(void *frame_addr, size_t n);

/**
 * Get the total phyiscal memory size available for the system. This is
 * not necessarily the total installed memory size.
 * 
 * @return The total phyiscal memory size in bytes.
 */
size_t pmm_get_total_memory_size();

/**
 * Get the size of the phyiscal free memory.
 * 
 * @return The size of the available/free phyiscal memory in bytes.
 */
size_t pmm_get_available_memory_size();

#endif // _KERNEL_MEMORY_PMM_H
