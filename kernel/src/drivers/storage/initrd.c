#include <drivers/storage/initrd.h>
#include <device/device.h>
#include <memory/kheap.h>
#include <system/kpanic.h>

static void* initrd_base;
static size_t initrd_size;

static size_t initrd_total_size(struct device* device);
static size_t initrd_sector_size(struct device* device);

int32_t initrd_init(void* memory_base, size_t memory_size) {
    uint16_t* initrd_header = (uint16_t*) memory_base;

    if(*initrd_header != INITRD_HEADER_MAGIC) {
        return -1;
    }

    initrd_base = (void*) (((uintptr_t) memory_base) + sizeof(uint16_t));
    initrd_size = memory_size - sizeof(uint16_t);

    storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->name = (char*) kmalloc(17);

    if(!device->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_generate_id(device->id);
    strcpy(device->name, "Initial Ramdisk");
    device->type = DEVICE_TYPE_STORAGE;
    device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
    device->bus.data = NULL;

    device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

    if(!device->driver.storage) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->driver.storage->sector_size = initrd_sector_size;
    device->driver.storage->total_size = initrd_total_size;
    device->driver.storage->read = initrd_read;
    device->driver.storage->write = initrd_write;

    device_register(NULL, device);

    return 0;
}

static size_t initrd_total_size(struct device* device) {
    (void) device;

    return initrd_size;
}

static size_t initrd_sector_size(struct device* device) {
    (void) device;

    // Useless for the initial ramdisk because we're not handling it as a block device
    return 0;
}

size_t initrd_read(struct device* device, size_t offset, size_t size, char* buffer) {
    (void) device;

    if(offset > initrd_size) {
        return 0;
    }

    if(offset + size > initrd_size) {
        size = initrd_size - offset;
    }

    memcpy(buffer, (void*) (((uintptr_t) initrd_base) + offset), size);

    return size;
}

size_t initrd_write(struct device* device, size_t offset, size_t size, char* buffer) {
    (void) device;
    (void) offset;
    (void) size;
    (void) buffer;

    // The initial ramdisk is read-only
    return 0;
}
