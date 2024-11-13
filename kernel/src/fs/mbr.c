#include <fs/mbr.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

bool mbr_probe(storage_device_t* device) {
    mbr_t mbr;

    device->driver->read(0, sizeof(mbr), (uint8_t*) &mbr);

    if(mbr.signature != MBR_SIGNATURE) {
        return false;
    }

    /*
     * You might think that the signature is enough to identify the Master Boot Record (MBR).
     * In fact, the MBR shares the signature with the Volume Boot Record (VBR) of certain file
     * systems/partitions such as FAT. In order to be able to determine whether it is an MBR
     * or just an unpartitioned disk with a FAT file system and thus VBR in the first sector,
     * further checks must be carried out.
     */

    // Make sure each partition has sensible properties
    for(int i = 0; i < 4; i++) {
        mbr_partition_entry_t* partition = &mbr.partitions[i];

        // Should be 0x80 for active/bootable or 0x00 for inactive
        if(partition->attributes != 0x80 && partition->attributes != 0x00) {
            return false;
        }
    }

    return true;
}

mbr_t* mbr_read(storage_device_t* device) {
    mbr_t* mbr = (mbr_t*) kmalloc(sizeof(mbr_t));

    if(!mbr) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->driver->read(0, sizeof(mbr_t), (uint8_t*) mbr);

    return mbr;
}
