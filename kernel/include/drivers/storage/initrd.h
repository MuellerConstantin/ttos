#ifndef _KERNEL_DRIVERS_STORAGE_INITRD_H
#define _KERNEL_DRIVERS_STORAGE_INITRD_H

#include <stdint.h>
#include <stddef.h>

#define INITRD_HEADER_MAGIC 0xCAFE

/**
 * Initializes the initial ramdisk.
 * 
 * @param memory_base The base address of the initial ramdisk.
 * @param memory_size The size of the initial ramdisk.
 * @return 0 if the initial ramdisk was successfully initialized, -1 otherwise.
 */
int32_t initrd_init(void* memory_base, size_t memory_size);

/**
 * Reads data from the initial ramdisk.
 * 
 * @param offset The offset from which to read data.
 * @param size The size of the data to read.
 * @param buffer The buffer to which to read the data.
 * @return The number of bytes read.
 */
size_t initrd_read(size_t offset, size_t size, void* buffer);

/**
 * Writes data to the initial ramdisk. This function is only for
 * completeness and will always return 0. The initial ramdisk is
 * read-only.
 * 
 * @param offset The offset to which to write data.
 * @param size The size of the data to write.
 * @param buffer The buffer from which to write the data.
 * @return The number of bytes written.
 */
size_t initrd_write(size_t offset, size_t size, void* buffer);

#endif // _KERNEL_DRIVERS_STORAGE_INITRD_H
