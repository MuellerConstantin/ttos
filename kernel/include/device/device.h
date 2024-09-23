#ifndef _KERNEL_DEVICE_DEVICE_H
#define _KERNEL_DEVICE_DEVICE_H

#include <stdint.h>
#include <string.h>
#include <ds/generic_tree.h>
#include <ds/linked_list.h>
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

typedef struct device device_t;
typedef struct bus bus_t;
typedef struct driver driver_t;

struct bus {
    uint8_t type;
    void* data;
};

struct device {
    char* name;
    uint16_t type;
    bus_t bus;

    union {
        keyboard_driver_t* keyboard;
        storage_driver_t* storage;
        video_driver_t* video;
    } driver;
};

/**
 * Initialize the device manager.
 */
void device_init();

/**
 * Get the device tree.
 * 
 * @return The device tree.
 */
generic_tree_t* device_get_tree();

/**
 * Register a device.
 * 
 * @param parent The parent device.
 * @param device The device to register.
 * @return The new device.
 */
void device_register(device_t* parent, device_t* device);

/**
 * Unregister a device.
 * 
 * @param device The device to unregister.
 */
void device_unregister(device_t* device);

/**
 * Find a device by type.
 * 
 * @param type The device type.
 * @return The device.
 */
device_t* device_find_by_type(uint16_t type);

/**
 * Find all devices by type.
 * 
 * @param type The device type.
 * @return The devices.
 */
linked_list_t* device_find_all_by_type(uint16_t type);

#endif // _KERNEL_DEVICE_DEVICE_H
