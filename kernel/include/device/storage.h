#ifndef _KERNEL_DEVICE_STORAGE_H
#define _KERNEL_DEVICE_STORAGE_H

#include <stdint.h>
#include <stddef.h>

typedef struct storage_driver storage_driver_t;

struct storage_driver {
    size_t (*total_size)();
    size_t (*read)(size_t offset, size_t size, char* buffer);
    size_t (*write)(size_t offset, size_t size, char* buffer);
};

#endif // _KERNEL_DEVICE_STORAGE_H
