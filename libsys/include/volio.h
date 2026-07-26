#ifndef _LIBSYS_VOLIO_H
#define _LIBSYS_VOLIO_H

#include <stdint.h>
#include <stddef.h>

typedef struct volinfo volinfo_t;

struct volinfo {
    char name[64];
    char id[16];
};

/**
 * Queries a volume by its index in the system's volume list.
 *
 * Callers enumerate all volumes by invoking this with index 0, 1, 2, ...
 * until it returns -1.
 *
 * @param index The index of the volume to query.
 * @param info The volume information to fill.
 * @return 0 on success or -1 when the index is out of range or on error.
 */
int32_t volio_list(uint32_t index, volinfo_t* info);

#endif // _LIBSYS_VOLIO_H
