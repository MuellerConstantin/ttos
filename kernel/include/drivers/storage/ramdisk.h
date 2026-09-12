/**
 * @file ramdisk.h
 * @brief Storage devices backed by memory.
 *
 * A RAM disk presents a region of kernel memory as a storage device. The region has to be mapped
 * before it is registered and stays the disk's backing store for as long as the device exists; the
 * driver copies into and out of it and owns nothing else. Multiboot modules are registered this way,
 * the initial ramdisk among them.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_RAMDISK_H
#define _KERNEL_DRIVERS_STORAGE_RAMDISK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct ramdisk ramdisk_t;

/** The backing store of one RAM disk, hung off its device as bus data. */
struct ramdisk {
    void* base;
    size_t size;
    bool read_only;
};

/**
 * Register a memory region as a storage device on the platform bus.
 *
 * @param name The device name, copied.
 * @param base The mapped start of the region.
 * @param size The size of the region in bytes.
 * @param read_only Whether writes are refused. A refused write reports zero bytes written.
 * @return 0 on success, otherwise an error code.
 */
int32_t ramdisk_register(const char* name, void* base, size_t size, bool read_only);

#endif // _KERNEL_DRIVERS_STORAGE_RAMDISK_H
