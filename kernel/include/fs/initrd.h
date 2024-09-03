#ifndef _KERNEL_FS_INITRD_H
#define _KERNEL_FS_INITRD_H

#include <stdint.h>
#include <fs/vfs.h>
#include <fs/mount.h>
#include <memory/kheap.h>

#define INITRD_HEADER_MAGIC 0xCAFE
#define INITRD_FILE_HEADER_MAGIC 0xBEEF

struct initrd_header {
    uint16_t magic;
    uint32_t n_files;
} __attribute__((packed));

typedef struct initrd_header initrd_header_t;

struct initrd_file_header {
    uint16_t magic;
    uint8_t name[64];
    uint32_t offset;
    uint32_t length;
} __attribute__((packed));

typedef struct initrd_file_header initrd_file_header_t;

/**
 * Initialize the initial ramdisk.
 * 
 * @param memory_base The base address of the memory.
 * @return The volume of the initial ramdisk.
 */
mnt_volume_t* initrd_init(void* memory_base);

#endif // _KERNEL_FS_INITRD_H