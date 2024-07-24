#ifndef _KERNEL_IO_VFS_H
#define _KERNEL_IO_VFS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#define VFS_MAX_MOUNTPOINTS 26
#define VFS_MAX_PATH_LENGTH 4096

#define DRIVE_A 'A'
#define DRIVE_B 'B'
#define DRIVE_C 'C'
#define DRIVE_D 'D'
#define DRIVE_E 'E'
#define DRIVE_F 'F'
#define DRIVE_G 'G'
#define DRIVE_H 'H'
#define DRIVE_I 'I'
#define DRIVE_J 'J'
#define DRIVE_K 'K'
#define DRIVE_L 'L'
#define DRIVE_M 'M'
#define DRIVE_N 'N'
#define DRIVE_O 'O'
#define DRIVE_P 'P'
#define DRIVE_Q 'Q'
#define DRIVE_R 'R'
#define DRIVE_S 'S'
#define DRIVE_T 'T'
#define DRIVE_U 'U'
#define DRIVE_V 'V'
#define DRIVE_W 'W'
#define DRIVE_X 'X'
#define DRIVE_Y 'Y'
#define DRIVE_Z 'Z'

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_SYMLINK     0x03

typedef struct vfs_node vfs_node_t;
typedef struct vfs_mount vfs_mount_t;
typedef struct vfs_dirent vfs_dirent_t;

struct vfs_node {
    char name[128];
    uint32_t permissions;
    uint32_t type;
    uint32_t uid;
    uint32_t gid;
    uint32_t inode;
    uint32_t length;
    struct vfs_node *link;

    int32_t (*read)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
    int32_t (*write)(vfs_node_t* node, uint32_t offset, size_t size, void* buffer);
    int32_t (*open)(vfs_node_t* node);
    int32_t (*close)(vfs_node_t* node);
    vfs_dirent_t* (*readdir)(vfs_node_t* node, uint32_t index);
    vfs_node_t* (*finddir)(vfs_node_t* node, char* name);
} __attribute__((packed));

struct vfs_mount {
    vfs_node_t* root;

    int32_t (*mount)(struct vfs_mount* mount);
    int32_t (*unmount)(struct vfs_mount* mount);
} __attribute__((packed));

struct vfs_dirent {
    char name[128];
    uint32_t inode;
} __attribute__((packed));

/**
 * Mount a file system.
 * 
 * @param drive The drive letter to mount the file system to.
 * @param mountpoint The mount point to mount.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_mount(char drive, vfs_mount_t* mountpoint);

/**
 * Unmount a file system.
 * 
 * @param drive The drive letter to unmount.
 * @return 0 on success or -1 on error.
 */
int32_t vfs_unmount(char drive);

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
 * Find a file node by an absolute path.
 * 
 * @param path The path to the file.
 * @return The file or NULL if not found.
 */
vfs_node_t* vfs_findpath(char* path);

#endif // _KERNEL_IO_VFS_H
