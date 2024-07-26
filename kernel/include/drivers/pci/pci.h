#ifndef _KERNEL_DRIVERS_PCI_H
#define _KERNEL_DRIVERS_PCI_H

#include <stdint.h>
#include <io/ports.h>
#include <drivers/pci/types.h>
#include <common/linked_list.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAX_NUM_BUSES 256
#define PCI_DEVICES_PER_BUS 32
#define PCI_FUNCTIONS_PER_DEVICE 8
#define PCI_MAX_DEFAULT_SCAN_BUSES 10

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08
#define PCI_PROG_IF 0x09
#define PCI_SUBCLASS 0x0A
#define PCI_CLASS 0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER 0x0D
#define PCI_HEADER_TYPE 0x0E
#define PCI_BIST 0x0F
#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR2 0x18
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24
#define PCI_INTERRUPT_LINE 0x3C
#define PCI_SECONDARY_BUS 0x19

typedef struct pci_device pci_device_t;

struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t type;
    uint8_t subtype;
    uint8_t prog_if;
} __attribute__((packed));

/**
 * Initializes the PCI driver.
 * 
 * @return 0 if the PCI driver was initialized successfully, -1 otherwise.
 */
int32_t pci_init();

/**
 * Retrieves a list of all PCI devices.
 * 
 * @return A linked list of all PCI devices.
 */
linked_list_t* pci_get_devices();

/**
 * Returns a linked list of all PCI devices of the specified type and subtype.
 * 
 * @param type The type of the PCI devices to get.
 * @param subtype An optional subtype of the PCI devices to get. If set to negative, all
 * subtypes will be included.
 * @return A linked list of all PCI devices of the specified type and subtype.
 */
linked_list_t* pci_get_devices_of_type(uint8_t type, int16_t subtype);

/**
 * Retrieves a PCI device with the specified vendor and device ID.
 * 
 * @param vendor_id The vendor ID of the PCI device.
 * @param device_id The device ID of the PCI device.
 * @return A pointer to the PCI device with the specified vendor and device ID, or NULL if no
 * such device was found.
 */
pci_device_t* pci_get_device(uint16_t vendor_id, uint16_t device_id);

#endif // _KERNEL_DRIVERS_PCI_H
