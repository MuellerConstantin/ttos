#ifndef _KERNEL_DEVICE_VOLUME_H
#define _KERNEL_DEVICE_VOLUME_H

#include <stdint.h>
#include <stdbool.h>
#include <device/device.h>
#include <fs/mount.h>
#include <linked_list.h>
#include <uuid.h>

typedef struct volume volume_t;

struct volume {
    uuid_t id;
    char* name;
    storage_device_t* device;
    mnt_mountpoint_t* mountpoint;
};

/**
 * Initialize the volume manager.
 */
void volume_init();

/**
 * Get the list of available volumes.
 * 
 * @return The list of volumes.
 */
linked_list_t* volume_get_all();

/**
 * Scan a device for volumes and register them.
 * 
 * @param device The device to scan.
 * @return The number of volumes found.
 */
size_t volume_register_device(storage_device_t* device);

/**
 * Unregister a device and all its volumes.
 * 
 * @param device The device to unregister.
 */
void volume_unregister_device(storage_device_t* device);

/**
 * Mount a volume.
 * 
 * @param drive The drive letter.
 * @param volume The volume to mount.
 * @return 0 on success or -1 on error.
 */
int32_t volume_mount(char drive, volume_t* volume);

/**
 * Unmount a volume.
 * 
 * @param volume The volume to unmount.
 * @return 0 on success or -1 on error.
 */
int32_t volume_unmount(volume_t* volume);

/**
 * Find a volume by its ID.
 * 
 * @param id The volume ID.
 * @return The volume or NULL if not found.
 */
volume_t* volume_find_by_id(uuid_t id);

#endif // _KERNEL_DEVICE_VOLUME_H
