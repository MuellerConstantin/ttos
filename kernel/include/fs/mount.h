#ifndef _KERNEL_FS_MOUNT_H
#define _KERNEL_FS_MOUNT_H

#include <stdbool.h>
#include <fs/vfs.h>

#define FS_VOLUME_MAX_MOUNTPOINTS 26

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

typedef struct mnt_volume mnt_volume_t;

typedef struct mnt_volume_operations mnt_volume_operations_t;

struct mnt_volume_operations {
    int32_t (*mount)(mnt_volume_t* mount);
    int32_t (*unmount)(mnt_volume_t* mount);
} __attribute__((packed));

struct mnt_volume {
    vfs_node_t* root;
    mnt_volume_operations_t* operations;
} __attribute__((packed));

/**
 * Mount a volume/file system.
 * 
 * @param drive The drive letter to mount the file system to.
 * @param volume The volume to mount.
 * @return 0 on success or -1 on error.
 */
int32_t mnt_volume_mount(char drive, mnt_volume_t* volume);

/**
 * Unmount a volume/file system.
 * 
 * @param drive The drive letter to unmount.
 * @return 0 on success or -1 on error.
 */
int32_t mnt_volume_unmount(char drive);

/**
 * Get the mounted volume for a path.
 * 
 * @param path The path to get the mounted volume for.
 * @return The mounted volume or NULL if not found.
 */
mnt_volume_t* mnt_get_mountpoint(char* path);

#endif // _KERNEL_FS_MOUNT_H
