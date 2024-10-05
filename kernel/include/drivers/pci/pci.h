#ifndef _KERNEL_DRIVERS_PCI_H
#define _KERNEL_DRIVERS_PCI_H

#include <stdint.h>
#include <io/ports.h>
#include <device/device.h>
#include <drivers/pci/types.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_MAX_NUM_BUSES 256
#define PCI_DEVICES_PER_BUS 32
#define PCI_FUNCTIONS_PER_DEVICE 8

#define PCI_HEADER_TYPE_0 0x00
#define PCI_HEADER_TYPE_1 0x01
#define PCI_HEADER_TYPE_2 0x02

// PCI common header fields

#define PCI_VENDOR_ID           0x00
#define PCI_DEVICE_ID           0x02
#define PCI_COMMAND             0x04
#define PCI_STATUS              0x06
#define PCI_REVISION_ID         0x08
#define PCI_PROG_IF             0x09
#define PCI_SUBCLASS            0x0A
#define PCI_CLASS               0x0B
#define PCI_CACHE_LINE_SIZE     0x0C
#define PCI_LATENCY_TIMER       0x0D
#define PCI_HEADER_TYPE         0x0E
#define PCI_BIST                0x0F

// PCI header type 0 fields

#define PCI_H0_BAR0             0x10
#define PCI_H0_BAR1             0x14
#define PCI_H0_BAR2             0x18
#define PCI_H0_BAR3             0x1C
#define PCI_H0_BAR4             0x20
#define PCI_H0_BAR5             0x24
#define PCI_H0_INTERRUPT_LINE   0x3C
#define PCI_H0_SECONDARY_BUS    0x19

struct pci_general_device_data {
    uint32_t bar[6];
    size_t bar_size[6];
    uint8_t interrupt_line;
};

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

    union {
        struct pci_general_device_data general;
    } data;
} __attribute__((packed));

/**
 * Initializes the PCI driver.
 * 
 * @return 0 if the PCI driver was initialized successfully, -1 otherwise.
 */
int32_t pci_init();

/**
 * Get a BAR value from a PCI device.
 * 
 * @param pci_device The PCI device.
 * @param bar The BAR number. Must be between 0 and 5.
 * @return The BAR value.
 */
uint32_t pci_get_bar_address(pci_device_t* pci_device, uint8_t bar_index);

/**
 * Get the size of a BAR from a PCI device.
 * 
 * @param pci_device The PCI device.
 * @param bar The BAR number. Must be between 0 and 5.
 * @return The size of the BAR.
 */
size_t pci_get_bar_size(pci_device_t* pci_device, uint8_t bar_index);

/**
 * Get the interrupt line of a PCI device.
 * 
 * @param pci_device The PCI device.
 * @return The interrupt line.
 */
uint8_t pci_get_interrupt_line(pci_device_t* pci_device);

#endif // _KERNEL_DRIVERS_PCI_H
