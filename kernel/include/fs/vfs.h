/**
 * @file vfs.h
 * @brief Virtual File System (VFS) interface.
 * 
 * The Virtual File System (VFS) interface provides a common interface for file systems. It defines
 * structures for nodes and directory entries, as well as operations for reading, writing, creating,
 * and deleting files and directories.
 */

#ifndef _KERNEL_FS_VFS_H
#define _KERNEL_FS_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <util/string.h>
#include <stdbool.h>
#include <ctype.h>
#include <device/volume.h>

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_SYMLINK     0x03

typedef struct vfs_node vfs_node_t;
typedef struct vfs_dirent vfs_dirent_t;

typedef struct vfs_node_operations vfs_node_operations_t;

typedef struct vfs_filesystem_operations vfs_filesystem_operations_t;
typedef struct vfs_filesystem vfs_filesystem_t;

struct vfs_node_operations {
    int32_t (*open)(vfs_node_t* node);
    int32_t (*close)(vfs_node_t* node);
    int32_t (*rename)(vfs_node_t* node, char* new_name);

    // File specific operations

    int32_t (*read)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
    int32_t (*write)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
    vfs_node_t* (*create)(vfs_node_t* node, char* name, uint32_t permissions);
    int32_t (*unlink)(vfs_node_t* node, char* name);

    // Directory specific operations

    int32_t (*mkdir)(vfs_node_t* node, char* name, uint32_t permissions);
    int32_t (*rmdir)(vfs_node_t* node, char* name);
    vfs_dirent_t* (*readdir)(vfs_node_t* node, uint32_t index);
    vfs_node_t* (*finddir)(vfs_node_t* node, char* name);
} __attribute__((packed));

struct vfs_node {
    char* name;
    uint32_t permissions;
    uint32_t type;
    uint32_t uid;
    uint32_t gid;
    uint32_t length;
    uint32_t inode;
    void* inode_data;
    struct vfs_node *link;
    vfs_node_operations_t* operations;
    vfs_filesystem_t* filesystem;
} __attribute__((packed));

struct vfs_dirent {
    char name[128];
    uint32_t inode;
} __attribute__((packed));

struct vfs_filesystem_operations {
    int32_t (*mount)(vfs_filesystem_t* filesystem);
    int32_t (*unmount)(vfs_filesystem_t* filesystem);
} __attribute__((packed));

struct vfs_filesystem {
    vfs_node_t* root;
    volume_t* volume;
    void* fs_data;
    vfs_filesystem_operations_t* operations;
} __attribute__((packed));

/**
 * Check if a path is an absolute path.
 * 
 * @param path The path to check.
 * @return True if the path is an absolute path, false otherwise.
 */
bool vfs_is_abs_path(char* path);

/**
 * Check if a node is a file.
 * 
 * @param node The node to check.
 * @return True if the node is a file, false otherwise.
 */
bool vfs_is_file(vfs_node_t* node);

/**
 * Check if a node is a directory.
 * 
 * @param node The node to check.
 * @return True if the node is a directory, false otherwise.
 */
bool vfs_is_dir(vfs_node_t* node);

/**
 * Open a node. This allows the underlying file system to load data into memory.
 * 
 * @param node The node to open.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_open(vfs_node_t* node);

/**
 * Close a node. This allows the underlying file system to unload data from memory.
 * 
 * @param node The node to close.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_close(vfs_node_t* node);

/**
 * Rename a node.
 * 
 * @param node The node to rename.
 * @param new_name The new name of the file/directory.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_rename(vfs_node_t* node, char* new_name);

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
 * Create a file.
 * 
 * @param node The directory to create the file in.
 * @param name The name of the file to create.
 * @param permissions The permissions of the file.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_create(vfs_node_t* node, char* name, uint32_t permissions);

/**
 * Delete a file.
 * 
 * @param node The directory to delete the file from.
 * @param name The name of the file to delete.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_unlink(vfs_node_t* node, char* name);

/**
 * Create a directory.
 * 
 * @param node The directory to create the new directory in.
 * @param name The name of the new directory.
 * @param permissions The permissions of the new directory.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_mkdir(vfs_node_t* node, char* name, uint32_t permissions);

/**
 * Delete a directory.
 * 
 * @param node The directory to delete the directory from.
 * @param name The name of the directory to delete.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_rmdir(vfs_node_t* node, char* name);

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
