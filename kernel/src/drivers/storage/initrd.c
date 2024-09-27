#include <drivers/storage/initrd.h>
#include <device/device.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

static void* initrd_base;
static size_t initrd_size;

static size_t initrd_total_size();

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

    device->info.name = (char*) kmalloc(17);

    if(!device->info.name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    generate_uuid_v4(&device->info.id);
    strcpy(device->info.name, "Initial Ramdisk");
    device->info.type = DEVICE_TYPE_STORAGE;
    device->info.bus.type = DEVICE_BUS_TYPE_PLATFORM;
    device->info.bus.data = NULL;

    device->driver = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

    if(!device->driver) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->driver->total_size = initrd_total_size;
    device->driver->read = initrd_read;
    device->driver->write = initrd_write;

    device_register(NULL, device);

    return 0;
}

static size_t initrd_total_size() {
    return initrd_size;
}

size_t initrd_read(size_t offset, size_t size, void* buffer) {
    if(offset > initrd_size) {
        return 0;
    }

    if(offset + size > initrd_size) {
        size = initrd_size - offset;
    }

    memcpy(buffer, (void*) (((uintptr_t) initrd_base) + offset), size);

    return size;
}

size_t initrd_write(size_t offset, size_t size, void* buffer) {
    // The initial ramdisk is read-only
    return 0;
}
