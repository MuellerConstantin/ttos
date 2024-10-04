/**
 * @file device.h
 * @brief Kernel's central device manager.
 * 
 * The device manager is responsible for managing all devices in the system. It provides
 * a common device interface for all devices and enables easy access to devices by
 * defining driver interfaces for different kind of devices.
 */

#ifndef _KERNEL_DEVICE_DEVICE_H
#define _KERNEL_DEVICE_DEVICE_H

#include <stdint.h>
#include <string.h>
#include <uuid.h>
#include <generic_tree.h>
#include <linked_list.h>
#include <device/keyboard.h>
#include <device/storage.h>
#include <device/video.h>

// Bus types

#define DEVICE_BUS_TYPE_PLATFORM    0x00
#define DEVICE_BUS_TYPE_ISA         0x01
#define DEVICE_BUS_TYPE_PCI         0x02
#define DEVICE_BUS_TYPE_USB         0x03
#define DEVICE_BUS_TYPE_RESERVED    0xFF

// Device types

#define DEVICE_TYPE_UNKNOWN     0x0000
#define DEVICE_TYPE_KEYBOARD    0x0100
#define DEVICE_TYPE_STORAGE     0x0200
#define DEVICE_TYPE_VIDEO       0x0300
#define DEVICE_TYPE_RESERVED    0xFF00

typedef struct bus bus_t;
typedef struct device device_t;
typedef struct video_device video_device_t;
typedef struct storage_device storage_device_t;
typedef struct keyboard_device keyboard_device_t;

/**
 * Structure representing a bus, used to connect devices.
 */
struct bus {
    uint8_t type;
    void* data;
};

/**
 * Common device structure, holding essential information about a device.
 */
struct device {
    uuid_t id;
    char* name;
    uint16_t type;
    bus_t bus;
} __attribute__((packed));

/**
 * Structure representing a video device.
 */
struct video_device {
    device_t info;
    video_driver_t* driver;
} __attribute__((packed));

/**
 * Structure representing a storage device.
 */
struct storage_device {
    device_t info;
    storage_driver_t* driver;
} __attribute__((packed));

/**
 * Structure representing a keyboard device.
 */
struct keyboard_device {
    device_t info;
    keyboard_driver_t* driver;
} __attribute__((packed));

/**
 * Initialize the device manager.
 */
void device_init();

/**
 * Get the device tree.
 * 
 * @return The device tree.
 */
const generic_tree_t* device_get_all();

/**
 * Register a device.
 * 
 * @param parent The parent device.
 * @param device The device to register.
 */
void device_register(device_t* parent, device_t* device);

/**
 * Unregister a device.
 * 
 * @param device The device to unregister.
 */
void device_unregister(device_t* device);

/**
 * Find a device by ID.
 * 
 * @param id The device ID.
 * @return The device.
 */
const device_t* device_find_by_id(uuid_t id);

/**
 * Find the first device by type.
 * 
 * @param type The device type.
 * @return The device or NULL if not found.
 */
const device_t* device_find_by_type(uint16_t type);

/**
 * Find a device by name.
 * 
 * @param name The device name.
 * @return The device or NULL if not found.
 */
const device_t* device_find_by_name(const char* name);

/**
 * Find all devices by type.
 * 
 * @param type The device type.
 * @return The devices.
 */
const linked_list_t* device_find_all_by_type(uint16_t type);

#endif // _KERNEL_DEVICE_DEVICE_H
