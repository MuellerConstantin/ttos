#ifndef _KERNEL_FS_MBR_H
#define _KERNEL_FS_MBR_H

#include <stdint.h>
#include <stdbool.h>
#include <device/device.h>

typedef struct mbr mbr_t;
typedef struct mbr_partition_entry mbr_partition_entry_t;

struct mbr_partition_entry {
    uint8_t attributes;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t sectors;
} __attribute__((packed));

struct mbr {
    uint8_t bootstrap[446];
    mbr_partition_entry_t partitions[4];
    uint16_t signature;
} __attribute__((packed));

/**
 * Probe a device for an existing MBR.
 * 
 * @param device The device to probe.
 * @return true if an MBR was found, false otherwise.
 */
bool mbr_probe(storage_device_t* device);

/**
 * Read the MBR from a device.
 * 
 * @param device The device to read from.
 * @return The MBR.
 */
mbr_t* mbr_read(storage_device_t* device);

#endif // _KERNEL_FS_MBR_H
