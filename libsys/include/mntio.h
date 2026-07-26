#ifndef _LIBSYS_MNTIO_H
#define _LIBSYS_MNTIO_H

#include <stdint.h>
#include <stddef.h>

typedef struct mntinfo mntinfo_t;

struct mntinfo {
    char drive;
};

/**
 * Queries a mount point by its index in the system's mount table.
 *
 * Callers enumerate all mount points by invoking this with index 0, 1, 2, ...
 * until it returns -1.
 *
 * @param index The index of the mount point to query.
 * @param info The mount point information to fill.
 * @return 0 on success or -1 when the index is out of range or on error.
 */
int32_t mntio_list(uint32_t index, mntinfo_t* info);

#endif // _LIBSYS_MNTIO_H
