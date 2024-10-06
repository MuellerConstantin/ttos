/**
 * @file multiboot_util.h
 * @brief Utility functions for the Multiboot information structure.
 * 
 * This file contains utility functions for the Multiboot information structure. It's important to
 * note that we're talking about the Multiboot 1 information structure, not the Multiboot 2 one.
 */

#ifndef _KERNEL_MULTIBOOT_UTIL_H
#define _KERNEL_MULTIBOOT_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <multiboot.h>

/**
 * Get the total memory size from the multiboot info structure.
 * 
 * @param multiboot_info The multiboot info structure.
 * @return The total memory size in bytes.
 */
size_t multiboot_get_memory_size(multiboot_info_t *multiboot_info);

/**
 * Get the free memory size from the multiboot info structure.
 * 
 * @param multiboot_info The multiboot info structure.
 * @return The free memory size in bytes.
 */
size_t multiboot_get_free_memory_size(multiboot_info_t *multiboot_info);

#endif // _KERNEL_MULTIBOOT_UTIL_H
