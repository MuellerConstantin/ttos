#ifndef _LIBSYS_MEMMAP_H
#define _LIBSYS_MEMMAP_H

#include <stdint.h>
#include <stddef.h>

typedef struct memregion memregion_t;

struct memregion {
    uint32_t base;
    uint32_t length;
    uint32_t type;
};

/**
 * Queries a physical memory region by its index in the memory map.
 *
 * Callers enumerate all regions by invoking this with index 0, 1, 2, ...
 * until it returns -1.
 *
 * @param index The index of the memory region to query.
 * @param region The memory region to fill.
 * @return 0 on success or -1 when the index is out of range or on error.
 */
int32_t memmap_read(uint32_t index, memregion_t* region);

#endif // _LIBSYS_MEMMAP_H
