#ifndef _KERNEL_DRIVERS_DEVICE_H
#define _KERNEL_DRIVERS_DEVICE_H

#include <stdint.h>
#include <string.h>
#include <ds/generic_tree.h>

#define DEVICE_BUS_TYPE_MOTHERBOARD 0x00
#define DEVICE_BUS_TYPE_ISA 0x01
#define DEVICE_BUS_TYPE_PCI 0x02
#define DEVICE_BUS_TYPE_USB 0x03
#define DEVICE_BUS_TYPE_RESERVED 0xFF

#define DEVICE_MAJOR_TYPE(value) (value & 0xFF00)
#define DEVICE_MINOR_TYPE(value) (value & 0x00FF)

// Major device types

#define DEVICE_TYPE_UNKNOWN 0x0000
#define DEVICE_TYPE_PROCESSOR 0x0100
#define DEVICE_TYPE_MEMORY 0x0200
#define DEVICE_TYPE_BUS 0x0300
#define DEVICE_TYPE_INPUT 0x0400
#define DEVICE_TYPE_STORAGE 0x0500
#define DEVICE_TYPE_NETWORK 0x0600
#define DEVICE_TYPE_DISPLAY 0x0700
#define DEVICE_TYPE_AUDIO 0x0800
#define DEVICE_TYPE_SERIAL 0x0900
#define DEVICE_TYPE_RESERVED 0xFF00

// Minor input device types

#define DEVICE_TYPE_KEYBOARD (DEVICE_TYPE_INPUT | 0x01)
#define DEVICE_TYPE_MOUSE (DEVICE_TYPE_INPUT | 0x02)

// Minor storage device types

#define DEVICE_TYPE_HDD (DEVICE_TYPE_STORAGE | 0x01)
#define DEVICE_TYPE_SSD (DEVICE_TYPE_STORAGE | 0x02)

// Minor network device types

#define DEVICE_TYPE_ETHERNET (DEVICE_TYPE_NETWORK | 0x01)
#define DEVICE_TYPE_WIFI (DEVICE_TYPE_NETWORK | 0x02)

// Minor display device types

#define DEVICE_TYPE_VGA (DEVICE_TYPE_DISPLAY | 0x01)
#define DEVICE_TYPE_VBE (DEVICE_TYPE_DISPLAY | 0x02)

// Minor audio device types

#define DEVICE_TYPE_SPEAKER (DEVICE_TYPE_AUDIO | 0x01)
#define DEVICE_TYPE_MICROPHONE (DEVICE_TYPE_AUDIO | 0x02)

// Minor serial device types

#define DEVICE_TYPE_UART (DEVICE_TYPE_SERIAL | 0x01)

typedef struct device device_t;

struct device {
    char* name;
    uint16_t device_type;
    uint8_t bus_type;
    void* bus_data;
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

#endif // _KERNEL_DRIVERS_DEVICE_H
