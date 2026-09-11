/**
 * @file mount.h
 * @brief Definitions for the file system mount manager.
 * 
 * The mount manager is responsible for managing mounted file systems. It provides a common interface
 * for mounting and unmounting file systems and enables easy access to mounted volumes by defining
 * mount operations for different kind of file systems.
 */

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

/**
 * Mount a volume by trying to detect an appropriate file system.
 * 
 * @param drive The drive letter to mount the file system to.
 * @param volume The volume to mount.
 * @return 0 on success or -1 on error.
 */
int32_t mnt_volume_mount(char drive, volume_t* volume);

/**
 * Unmount a volume/file system. A locked drive is refused.
 * 
 * @param drive The drive letter to unmount.
 * @return 0 on success or -1 on error.
 */
int32_t mnt_volume_unmount(char drive);

/**
 * Lock a mounted drive so that it cannot be unmounted. There is no way to
 * release the lock; it marks a drive the system cannot run without.
 *
 * @param drive The drive letter to lock.
 * @return 0 on success or -1 if the drive is not mounted.
 */
int32_t mnt_drive_lock(char drive);

/**
 * Check whether a drive is locked against unmounting.
 *
 * @param drive The drive letter to check.
 * @return True if the drive is locked, false otherwise.
 */
bool mnt_drive_is_locked(char drive);

/**
 * Get the mounted filesystem/volume for a path.
 * 
 * @param path The path to get the mounted volume for.
 * @return The mounted filesystem/volume or NULL if not found.
 */
const vfs_filesystem_t* mnt_get_mountpoint(char* path);

/**
 * Get the mounted filesystem/volume for a drive.
 * 
 * @param drive The drive letter to get the mounted volume for.
 * @return The mounted filesystem/volume or NULL if not found.
 */
const vfs_filesystem_t* mnt_get_drive(char drive);

/**
 * Get the drive a volume is mounted to.
 *
 * @param volume The volume to look up.
 * @return The drive letter or 0 if the volume is not mounted.
 */
char mnt_get_volume_drive(const volume_t* volume);

#endif // _KERNEL_FS_MOUNT_H
