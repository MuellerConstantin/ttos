#ifndef _KERNEL_IO_VFS_H
#define _KERNEL_IO_VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHARDEVICE  0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE        0x05
#define VFS_SYMLINK     0x06
#define VFS_MOUNTPOINT  0x08

typedef struct vfs_node vfs_node_t;
typedef struct vfs_dirent vfs_dirent_t;

typedef int32_t (*vfs_read_t)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
typedef int32_t (*vfs_write_t)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
typedef int32_t (*vfs_open_t)(vfs_node_t* node);
typedef int32_t (*vfs_close_t)(vfs_node_t* node);
typedef vfs_dirent_t* (*vfs_readdir_t)(vfs_node_t* node, uint32_t index);
typedef vfs_node_t* (*vfs_finddir_t)(vfs_node_t* node, char* name);

struct vfs_node {
    char name[128];
    uint32_t permissions;
    uint32_t flags;
    uint32_t uid;
    uint32_t gid;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;

    vfs_read_t read;
    vfs_write_t write;
    vfs_open_t open;
    vfs_close_t close;
    vfs_readdir_t readdir;
    vfs_finddir_t finddir;

    struct vfs_node *ptr;
} __attribute__((packed));

struct vfs_dirent {
    char name[128];
    uint32_t inode;
} __attribute__((packed));

/**
 * Mount the root filesystem.
 * 
 * @param node The root filesystem node.
 */
void vfs_root_mount(vfs_node_t* node);

/**
 * Get the root filesystem node.
 * 
 * @return The root filesystem node.
 */
vfs_node_t* vfs_get_root();

/**
 * Read data from a file.
 * 
 * @param node The file to read from.
 * @param offset The offset to start reading from.
 * @param size The number of bytes to read.
 * @param buffer The buffer to read the data into.
 * @return The number of bytes read or -1 on error.
 */
int32_t vfs_read(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);

/**
 * Write data to a file.
 * 
 * @param node The file to write to.
 * @param offset The offset to start writing to.
 * @param size The number of bytes to write.
 * @param buffer The buffer to write the data from.
 * @return The number of bytes written or -1 on error.
 */
int32_t vfs_write(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);

/**
 * Open a file.
 * 
 * @param node The file to open.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_open(vfs_node_t* node);

/**
 * Close a file.
 * 
 * @param node The file to close.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_close(vfs_node_t* node);

/**
 * Read a directory entry.
 * 
 * @param node The directory to read from.
 * @param index The index of the entry to read.
 * @return The directory entry or NULL on error.
 */
vfs_dirent_t* vfs_readdir(vfs_node_t* node, uint32_t index);

/**
 * Find a file in a directory.
 * 
 * @param node The directory to search.
 * @param name The name of the entry to find.
 * @return The file or NULL if not found.
 */
vfs_node_t* vfs_finddir(vfs_node_t* node, char* name);

#endif // _KERNEL_IO_VFS_H
