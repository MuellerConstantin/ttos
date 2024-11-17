/**
 * @file dir.h
 * @brief Directory handling operations.
 * 
 * This file contains functions for handling directories, such as reading directory entries and finding
 * directories independently of the underlying file system.
 */

#ifndef _KERNEL_IO_DIR_H
#define _KERNEL_IO_DIR_H

#include <stdint.h>
#include <stdbool.h>
#include <fs/vfs.h>

#define MAX_DIR_DESCRIPTORS 256

typedef struct dir_descriptor dir_descriptor_t;

struct dir_descriptor {
    vfs_node_t* node;
    uint32_t index;
};

typedef struct dir_dirent dir_dirent_t;

struct dir_dirent {
    char name[256];
    uint32_t inode;
};

/**
 * Open a directory.
 * 
 * @param path The path to the directory.
 * @return The directory descriptor or -1 on error.
 */
int32_t dir_open(char* path);

/**
 * Close a directory.
 * 
 * @param dd The directory descriptor to close.
 * @return 0 on success or -1 on error.
 */
int32_t dir_close(int32_t dd);

/**
 * Read a directory entry.
 * 
 * @param dd The directory descriptor to read from.
 * @return The directory entry or NULL if there are no more entries.
 */
const dir_dirent_t* dir_read(int32_t dd);

#endif // _KERNEL_IO_DIR_H
