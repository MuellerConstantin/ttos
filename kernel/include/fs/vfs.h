#ifndef _KERNEL_FS_VFS_H
#define _KERNEL_FS_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_SYMLINK     0x03

typedef struct vfs_node vfs_node_t;
typedef struct vfs_dirent vfs_dirent_t;

typedef struct vfs_node_operations vfs_node_operations_t;

struct vfs_node_operations {
    int32_t (*read)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
    int32_t (*write)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
    int32_t (*open)(vfs_node_t* node);
    int32_t (*close)(vfs_node_t* node);
    vfs_node_t* (*create)(vfs_node_t* node, char* name, uint32_t permissions);
    int32_t (*unlink)(vfs_node_t* node, char* name);
    int32_t (*mkdir)(vfs_node_t* node, char* name, uint32_t permissions);
    int32_t (*rmdir)(vfs_node_t* node, char* name);
    int32_t (*rename)(vfs_node_t* node, char* old_name, char* new_name);
    vfs_dirent_t* (*readdir)(vfs_node_t* node, uint32_t index);
    vfs_node_t* (*finddir)(vfs_node_t* node, char* name);
} __attribute__((packed));

struct vfs_node {
    char name[128];
    uint32_t permissions;
    uint32_t type;
    uint32_t uid;
    uint32_t gid;
    uint32_t inode;
    uint32_t length;
    struct vfs_node *link;
    vfs_node_operations_t* operations;
} __attribute__((packed));

struct vfs_dirent {
    char name[128];
    uint32_t inode;
} __attribute__((packed));

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

/**
 * Find a file by path relative to a node.
 * 
 * @param node The node to search from.
 * @param path The path to search for.
 * @return The file or NULL if not found.
 */
vfs_node_t* vfs_findpath(vfs_node_t* node, char* path);

#endif // _KERNEL_FS_VFS_H
