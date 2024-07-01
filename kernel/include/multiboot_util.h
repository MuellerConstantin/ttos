#ifndef _KERNEL_MULTIBOOT_UTIL_H
#define _KERNEL_MULTIBOOT_UTIL_H

#include <stdint.h>
#include <stddef.h>
#include <multiboot.h>

/**
 * Get the total memory size from the multiboot info structure.
 * 
 * @param multiboot_info The multiboot info structure.
 * @return The total memory size.
 */
size_t multiboot_get_memory_size(multiboot_info_t *multiboot_info);

#endif // _KERNEL_MULTIBOOT_UTIL_H
