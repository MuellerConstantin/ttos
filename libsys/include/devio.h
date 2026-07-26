#ifndef _LIBSYS_DEVIO_H
#define _LIBSYS_DEVIO_H

#include <stdint.h>
#include <stddef.h>

typedef struct devinfo devinfo_t;

struct devinfo {
    char name[64];
    char uuid[37];
};

/**
 * Queries a device by its index in the system's device tree.
 *
 * Callers enumerate all devices by invoking this with index 0, 1, 2, ...
 * until it returns -1.
 *
 * @param index The index of the device to query.
 * @param info The device information to fill.
 * @return 0 on success or -1 when the index is out of range or on error.
 */
int32_t devio_list(uint32_t index, devinfo_t* info);

#endif // _LIBSYS_DEVIO_H
