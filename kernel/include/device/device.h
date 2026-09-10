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
#include <util/string.h>
#include <util/uuid.h>
#include <util/shortid.h>
#include <util/generic_tree.h>
#include <util/linked_list.h>
#include <ttos/syscall.h>
#include <device/keyboard.h>
#include <device/storage.h>
#include <device/video.h>


typedef struct bus bus_t;
typedef struct device device_t;

/*
 * A device is a device regardless of what drives it. The aliases stay so that
 * declarations keep saying what kind of device they expect.
 */
typedef device_t video_device_t;
typedef device_t storage_device_t;
typedef device_t keyboard_device_t;

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
    char id[SHORT_ID_LENGTH + 1];
    char* name;
    uint16_t type;
    bus_t bus;

    union {
        void* raw;
        storage_driver_t* storage;
        video_driver_t* video;
        keyboard_driver_t* keyboard;
    } driver;
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
 * @return 0 if the device was registered successfully, -1 otherwise.
 */
int32_t device_register(device_t* parent, device_t* device);

/**
 * Unregister a device.
 * 
 * @param device The device to unregister.
 * @return 0 if the device was unregistered successfully, -1 otherwise.
 */
int32_t device_unregister(device_t* device);

/**
 * Find a device by its short ID.
 *
 * @param id The device ID.
 * @return The device.
 */
const device_t* device_find_by_id(const char* id);

/**
 * Generates a unique short ID for a device.
 *
 * @param buffer The buffer to write the id to. Must be at least SHORT_ID_LENGTH + 1 bytes.
 */
void device_generate_id(char* buffer);

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

/**
 * Find all devices by bus type.
 * 
 * @param bus_type The bus type.
 * @return The devices.
 */
const linked_list_t* device_find_all_by_bus_type(uint8_t bus_type);

#endif // _KERNEL_DEVICE_DEVICE_H
