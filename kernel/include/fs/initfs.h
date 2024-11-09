#ifndef _KERNEL_FS_INITFS_H
#define _KERNEL_FS_INITFS_H

#include <stdint.h>
#include <stdbool.h>
#include <fs/vfs.h>
#include <device/volume.h>

#define INITFS_HEADER_MAGIC 0xDEAD
#define INITFS_FILE_HEADER_MAGIC 0xBEEF

struct initfs_header {
    uint16_t magic;
    uint32_t n_files;
} __attribute__((packed));

typedef struct initfs_header initfs_header_t;

struct initfs_file_header {
    uint16_t magic;
    uint8_t name[64];
    uint32_t offset;
    uint32_t length;
} __attribute__((packed));

typedef struct initfs_file_header initfs_file_header_t;

/**
 * Probe for an initfs file system.
 * 
 * @param volume The volume to probe.
 * @return True if the file system is an initfs file system, false otherwise.
 */
bool initfs_probe(volume_t* volume);

/**
 * Initialize an initfs file system.
 * 
 * @param volume The volume to initialize.
 * @return The mount point or NULL on error.
 */
vfs_filesystem_t* initfs_init(volume_t* volume);

#endif // _KERNEL_FS_INITFS_H
