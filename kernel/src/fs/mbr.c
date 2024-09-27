#include <fs/mbr.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

bool mbr_probe(storage_device_t* device) {
    mbr_t mbr;

    device->driver->read(0, sizeof(mbr), (uint8_t*) &mbr);

    return mbr.signature == MBR_SIGNATURE;
}

mbr_t* mbr_read(storage_device_t* device) {
    mbr_t* mbr = (mbr_t*) kmalloc(sizeof(mbr_t));

    if(!mbr) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->driver->read(0, sizeof(mbr_t), (uint8_t*) mbr);

    return mbr;
}
