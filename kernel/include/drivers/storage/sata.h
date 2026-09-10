/**
 * @file sata.h
 * @brief A basic AHCI/SATA driver.
 * 
 * This file contains definitions for the SATA driver. So far the driver only detects a PCI
 * connected AHCI controller and maps its memory registers. Port enumeration is not implemented
 * yet, so no drive is registered with the device manager and no data can be read or written.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_SATA_H
#define _KERNEL_DRIVERS_STORAGE_SATA_H

#include <stdint.h>
#include <util/string.h>

/**
 * Initialize the SATA driver and detect the AHCI controller.
 *
 * @return 0 if the driver was initialized successfully, otherwise an error code.
 */
int32_t sata_init();

#endif // _KERNEL_DRIVERS_STORAGE_SATA_H
