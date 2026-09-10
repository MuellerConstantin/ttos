/**
 * @file storage.h
 * @brief Definitions for the storage devices.
 * 
 * This file contains structures and definitions for the storage devices. This includes the
 * storage driver interface and the storage device structure.
 */

#ifndef _KERNEL_DEVICE_STORAGE_H
#define _KERNEL_DEVICE_STORAGE_H

#include <stdint.h>
#include <stddef.h>

typedef struct storage_driver storage_driver_t;

/*
 * The device is declared rather than included: device.h pulls this file in for
 * the driver interface, so reaching back for the full definition here would
 * close a circle.
 */
struct device;

/**
 * What a driver has to offer for the storage devices it registers.
 *
 * Every call carries the device it is meant for. A driver that serves more than
 * one drive - and an IDE driver serves up to four per controller - would
 * otherwise need one set of functions per drive, each hard wired to the drive it
 * belongs to.
 */
struct storage_driver {
    size_t (*sector_size)(struct device* device);
    size_t (*total_size)(struct device* device);
    size_t (*read)(struct device* device, size_t offset, size_t size, char* buffer);
    size_t (*write)(struct device* device, size_t offset, size_t size, char* buffer);
};

#endif // _KERNEL_DEVICE_STORAGE_H
