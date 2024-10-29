/**
 * @file file.h
 * @brief File handling operations.
 * 
 * This file contains functions for handling files, such as opening, closing, reading, and writing files
 * independently of the underlying file system.
 */

#ifndef _KERNEL_IO_FILE_H
#define _KERNEL_IO_FILE_H

#include <stdint.h>
#include <stdbool.h>
#include <fs/vfs.h>

#define MAX_FILE_DESCRIPTORS 256

#define FILE_MODE_R         0b00000001
#define FILE_MODE_W         0b00000010

#define FILE_SEEK_CUR       0
#define FILE_SEEK_BEGIN     1
#define FILE_SEEK_END       2

typedef struct file_descriptor file_descriptor_t;

struct file_descriptor {
    vfs_node_t* node;
    uint32_t size;
    uint32_t offset;
    uint32_t flags;
};

typedef struct file_stat file_stat_t;

struct file_stat {
    size_t size;
    uint32_t uid;
    uint32_t gid;
    uint32_t permissions;
};

/**
 * Open a file.
 * 
 * @param path The path to the file.
 * @param flags The flags to open the file with.
 * @return The file descriptor or -1 on error.
 */
int32_t file_open(char* path, uint32_t flags);

/**
 * Close a file.
 * 
 * @param fd The file descriptor to close.
 * @return 0 on success or -1 on error.
 */
int32_t file_close(int32_t fd);

/**
 * Read from a file.
 * 
 * @param fd The file descriptor to read from.
 * @param buffer The buffer to read into.
 * @param size The number of bytes to read.
 * @return The number of bytes read or -1 on error.
 */
int32_t file_read(int32_t fd, void* buffer, size_t size);

/**
 * Write to a file.
 * 
 * @param fd The file descriptor to write to.
 * @param buffer The buffer to write from.
 * @param size The number of bytes to write.
 * @return The number of bytes written or -1 on error.
 */
int32_t file_write(int32_t fd, void* buffer, size_t size);

/**
 * Seek to a position in a file.
 * 
 * @param fd The file descriptor to seek in.
 * @param offset The offset to seek to.
 * @param whence The position to seek from.
 * @return 0 on success or -1 on error.
 */
int32_t file_seek(int32_t fd, int32_t offset, int32_t whence);

/**
 * Get the stat of a file.
 * 
 * @param path The path to the file.
 * @param stat The file stat to store the size in.
 * @return 0 on success or -1 on error.
 */
int32_t file_stat(const char* path, file_stat_t* stat);

#endif // _KERNEL_IO_FILE_H
