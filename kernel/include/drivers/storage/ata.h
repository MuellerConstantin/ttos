/**
 * @file ata.h
 * @brief A basic legacy IDE/ATA driver.
 * 
 * This file contains definitions for the ATA driver. The driver is capable of reading and writing
 * data from/to ATA drives. It talks to an on-board IDE controller, in compatibility mode through
 * the legacy ports and in native mode through the ports its BARs point at.
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

/*
 * Bits of the prog_if register of an IDE controller. A channel in compatibility
 * mode answers on the legacy ports above, one in native mode on the ports its
 * BARs were assigned: BAR0 and BAR1 for the primary channel, BAR2 and BAR3 for
 * the secondary one. Which of the two a channel uses is not a given - a chipset
 * that carries a PATA and a SATA controller side by side can only let one of
 * them have the legacy ports.
 */
#define ATA_PROG_IF_PRIMARY_NATIVE 0x01
#define ATA_PROG_IF_SECONDARY_NATIVE 0x04

/*
 * Upper bounds for the polling loops of the driver, counted in reads of the
 * status register. No wait may run forever: a port that is decoded by hardware
 * which is not a drive answers just enough to get past the probe and then never
 * finishes a command, and an unbounded wait turns that into a hung boot with
 * nothing on screen to say why.
 *
 * The probe gives up quickly because it runs up to four times per channel on
 * hardware that is not known to exist. A command is given a lot longer: there
 * is a drive at the other end by then, and it may well be busy.
 */
#define ATA_PROBE_TIMEOUT 100000
#define ATA_COMMAND_TIMEOUT 1000000

/** Length of the buffer the kernel message about a controller is built in. */
#define ATA_MESSAGE_LENGTH 128

/** Length of the buffer the name of a drive is built in. */
#define ATA_DRIVE_NAME_LENGTH 64

/** BARs holding the ports of a channel running in native mode. */
#define ATA_PRIMARY_COMMAND_BAR 0
#define ATA_PRIMARY_CONTROL_BAR 1
#define ATA_SECONDARY_COMMAND_BAR 2
#define ATA_SECONDARY_CONTROL_BAR 3

/*
 * The control block of a channel in native mode is four ports wide and the
 * device control register sits in its upper half, unlike the legacy control
 * port which is the register itself.
 */
#define ATA_CONTROL_BAR_OFFSET 2

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

/*
 * Stops the drive from raising interrupts. This driver polls, so it has no use
 * for them, and a drive left free to raise one is a hazard: in native mode the
 * interrupt travels a PCI line, which stays asserted until the drive is told
 * otherwise. An interrupt nobody is listening for is acknowledged at the
 * controller and immediately raised again, and the machine spends the rest of
 * its life in the interrupt.
 */
#define ATA_DEVICE_CONTROL_NIEN 0x02
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

typedef struct ata_channel ata_channel_t;

/** The ports one channel of a controller answers on. */
struct ata_channel {
    uint16_t io_base;
    uint16_t control_base;
};

struct ata_device {
    ata_drive_t drive;

    /** Ports of the channel this drive sits on. */
    uint16_t io_base;
    uint16_t control_base;

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
