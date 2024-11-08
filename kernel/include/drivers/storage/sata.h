/**
 * @file sata.h
 * @brief A basic AHCI/SATA driver.
 * 
 * This file contains definitions for the SATA driver. The driver is capable of reading and writing
 * data from/to SATA drives. It requires a PCI connected AHCI controller for detecting and accessing
 * SATA drives.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_SATA_H
#define _KERNEL_DRIVERS_STORAGE_SATA_H

#include <stdint.h>
#include <util/string.h>

/**
 * Initialize the SATA driver and detect the drives.
 * 
 * @return 0 if the driver was initialized successfully, otherwise an error code.
 */
int32_t sata_init();

#endif // _KERNEL_DRIVERS_STORAGE_SATA_H
