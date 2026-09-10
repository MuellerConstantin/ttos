/**
 * @file ata.h
 * @brief A basic legacy IDE/ATA driver.
 * 
 * This file contains definitions for the ATA driver. The driver is capable of reading and writing
 * data from/to ATA drives. It requires an on-board IDE controller in legacy mode for detecting and
 * accessing ATA drives.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_ATA_H
#define _KERNEL_DRIVERS_STORAGE_ATA_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <system/ports.h>

#define ATA_PRIMARY_IO_BASE 0x1F0
#define ATA_PRIMARY_CONTROL_BASE 0x3F6

#define ATA_SECONDARY_IO_BASE 0x170
#define ATA_SECONDARY_CONTROL_BASE 0x376

#define ATA_DATA_REGISTER 0x00
#define ATA_ERROR_REGISTER 0x01
#define ATA_FEATURES_REGISTER 0x01
#define ATA_SECTOR_COUNT_REGISTER 0x02
#define ATA_LBA_LOW_REGISTER 0x03
#define ATA_LBA_MID_REGISTER 0x04
#define ATA_LBA_HIGH_REGISTER 0x05
#define ATA_DRIVE_REGISTER 0x06
#define ATA_STATUS_REGISTER 0x07
#define ATA_COMMAND_REGISTER 0x07

#define ATA_STATUS_ERR 0x01     // An error occurred during the last command
#define ATA_STATUS_DRQ 0x08     // The drive is ready to transfer a block of data
#define ATA_STATUS_DF  0x20     // Drive fault
#define ATA_STATUS_BSY 0x80     // The drive is busy and the registers must not be touched

#define ATA_COMMAND_READ_SECTORS 0x20
#define ATA_COMMAND_WRITE_SECTORS 0x30
#define ATA_COMMAND_CACHE_FLUSH 0xE7

#define ATA_ALT_STATUS_REGISTER 0x00
#define ATA_DEVICE_CONTROL_REGISTER 0x00
#define ATA_DRIVE_ADDRESS_REGISTER 0x01

#define ATA_SECTOR_SIZE 512

/*
 * Upper bound for the drive size the driver is able to report. The storage
 * interface addresses a drive by byte offset in a size_t, which is 32 bit wide
 * on i386, so nothing beyond 4 GiB can be addressed at all. Reading and writing
 * is implemented with LBA28 only, which would allow 128 GiB, so the byte offset
 * is the binding limit. Capacities beyond it are reported clamped instead of
 * wrapped around.
 */
#define ATA_MAX_ADDRESSABLE_SECTORS (0xFFFFFFFFUL / ATA_SECTOR_SIZE)

typedef enum {
    ATA_PRIMARY_MASTER_DRIVE,
    ATA_PRIMARY_SLAVE_DRIVE,
    ATA_SECONDARY_MASTER_DRIVE,
    ATA_SECONDARY_SLAVE_DRIVE
} ata_drive_t;

typedef struct ata_device ata_device_t;

struct ata_device {
    ata_drive_t drive;
    bool present;
    bool lba_supported;
    bool lba48_supported;
    uint32_t size;
};

/**
 * Initialize the ATA driver and detect the drives.
 * 
 * @return 0 if the driver was initialized successfully, otherwise an error code.
 */
int32_t ata_init();

/**
 * Write data to an ATA drive.
 * 
 * @param device The device to write to.
 * @param offset The offset to write to.
 * @param size The size of the data to write.
 * @param buffer The buffer to write the data from.
 * @return The number of bytes written.
 */
size_t ata_write(ata_device_t* device, size_t offset, size_t size, char* buffer);

/**
 * Read data from an ATA drive.
 * 
 * @param device The device to read from.
 * @param offset The offset to read from.
 * @param size The size of the data to read.
 * @param buffer The buffer to read the data to.
 * @return The number of bytes read.
 */
size_t ata_read(ata_device_t* device, size_t offset, size_t size, char* buffer);

#endif // _KERNEL_DRIVERS_STORAGE_ATA_H
