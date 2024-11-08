/**
 * @file volume.h
 * @brief Kernel's volume manager.
 * 
 * The volume manager is responsible for managing all volumes in the system. It provides a common
 * volume interface for all volumes and enables easy access to volumes by defining volume operations
 * for different kind of volumes. Moreover, the volume manager is responsible for scanning storage
 * devices for volumes and registering them.
 */

#ifndef _KERNEL_DEVICE_VOLUME_H
#define _KERNEL_DEVICE_VOLUME_H

#include <stdint.h>
#include <stdbool.h>
#include <device/device.h>
#include <util/linked_list.h>
#include <util/uuid.h>

typedef struct volume volume_t;
typedef struct volume_operations volume_operations_t;

struct volume_operations {
    size_t (*total_size)(volume_t* volume);
    size_t (*read)(volume_t* volume, size_t offset, size_t size, char* buffer);
    size_t (*write)(volume_t* volume, size_t offset, size_t size, char* buffer);
};

struct volume {
    uuid_t id;
    char* name;
    size_t offset;
    size_t size;
    storage_device_t* device;
    volume_operations_t* operations;
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
const linked_list_t* volume_get_all();

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
 * Find a volume by its ID.
 * 
 * @param id The volume ID.
 * @return The volume or NULL if not found.
 */
const volume_t* volume_find_by_id(uuid_t id);

/**
 * Find a volume by its name.
 * 
 * @param name The volume name.
 * @return The volume or NULL if not found.
 */
const volume_t* volume_find_by_name(const char* name);

#endif // _KERNEL_DEVICE_VOLUME_H
