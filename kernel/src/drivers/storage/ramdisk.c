#include <drivers/storage/ramdisk.h>
#include <device/device.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <util/string.h>

static size_t ramdisk_total_size(struct device* device);
static size_t ramdisk_sector_size(struct device* device);
static size_t ramdisk_read(struct device* device, size_t offset, size_t size, char* buffer);
static size_t ramdisk_write(struct device* device, size_t offset, size_t size, char* buffer);

int32_t ramdisk_register(const char* name, void* base, size_t size, bool read_only) {
    ramdisk_t* ramdisk = (ramdisk_t*) kmalloc(sizeof(ramdisk_t));

    if(!ramdisk) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    ramdisk->base = base;
    ramdisk->size = size;
    ramdisk->read_only = read_only;

    storage_device_t* device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->name = (char*) kmalloc(strlen(name) + 1);

    if(!device->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device_generate_id(device->id);
    strcpy(device->name, name);
    device->type = DEVICE_TYPE_STORAGE;
    device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
    device->bus.data = ramdisk;

    device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

    if(!device->driver.storage) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->driver.storage->sector_size = ramdisk_sector_size;
    device->driver.storage->total_size = ramdisk_total_size;
    device->driver.storage->read = ramdisk_read;
    device->driver.storage->write = ramdisk_write;

    return device_register(NULL, device);
}

static size_t ramdisk_total_size(struct device* device) {
    return ((ramdisk_t*) device->bus.data)->size;
}

static size_t ramdisk_sector_size(struct device* device) {
    (void) device;

    /*
     * Memory has no sectors. The volume manager only needs a sector size to
     * place MBR partitions, and a RAM disk is expected to carry a file system
     * directly rather than a partition table.
     */
    return 0;
}

static size_t ramdisk_read(struct device* device, size_t offset, size_t size, char* buffer) {
    ramdisk_t* ramdisk = (ramdisk_t*) device->bus.data;

    if(offset > ramdisk->size) {
        return 0;
    }

    if(offset + size > ramdisk->size) {
        size = ramdisk->size - offset;
    }

    memcpy(buffer, (void*) ((uintptr_t) ramdisk->base + offset), size);

    return size;
}

static size_t ramdisk_write(struct device* device, size_t offset, size_t size, char* buffer) {
    ramdisk_t* ramdisk = (ramdisk_t*) device->bus.data;

    if(ramdisk->read_only || offset > ramdisk->size) {
        return 0;
    }

    if(offset + size > ramdisk->size) {
        size = ramdisk->size - offset;
    }

    memcpy((void*) ((uintptr_t) ramdisk->base + offset), buffer, size);

    return size;
}
