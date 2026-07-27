#ifndef _LIBSYS_MNTIO_H
#define _LIBSYS_MNTIO_H

#include <stdint.h>
#include <stddef.h>

// Return codes for mntio_mount.
#define MOUNT_OK              0
#define MOUNT_ERR_NOT_FOUND  -1
#define MOUNT_ERR_IN_USE     -2
#define MOUNT_ERR_FAILED     -3

// Return codes for mntio_unmount.
#define UNMOUNT_OK               0
#define UNMOUNT_ERR_NOT_MOUNTED -1
#define UNMOUNT_ERR_FAILED      -2

typedef struct mntinfo mntinfo_t;

struct mntinfo {
    char drive;
};

/**
 * Queries a mount point by its index in the system's mount table.
 *
 * Callers enumerate all mount points by invoking this with index 0, 1, 2, ...
 * until it returns -1.
 *
 * @param index The index of the mount point to query.
 * @param info The mount point information to fill.
 * @return 0 on success or -1 when the index is out of range or on error.
 */
int32_t mntio_list(uint32_t index, mntinfo_t* info);

/**
 * Mounts a volume to a drive.
 *
 * @param drive The drive letter to mount to.
 * @param id The short id of the volume to mount.
 * @return MOUNT_OK on success, or one of the MOUNT_ERR_* codes on failure.
 */
int32_t mntio_mount(char drive, const char* id);

/**
 * Unmounts a drive.
 *
 * @param drive The drive letter to unmount.
 * @return UNMOUNT_OK on success, or one of the UNMOUNT_ERR_* codes on failure.
 */
int32_t mntio_unmount(char drive);

#endif // _LIBSYS_MNTIO_H
