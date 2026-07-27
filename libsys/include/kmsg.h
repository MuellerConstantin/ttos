#ifndef _LIBSYS_KMSG_H
#define _LIBSYS_KMSG_H

#include <stdint.h>
#include <stddef.h>

typedef struct kmsg_entry kmsg_entry_t;

struct kmsg_entry {
    char level[16];
    char message[256];
};

/**
 * Reads a kernel log message by its index in the message log.
 *
 * Callers enumerate all messages by invoking this with index 0, 1, 2, ...
 * until it returns -1.
 *
 * @param index The index of the message to read.
 * @param entry The entry to fill.
 * @return 0 on success or -1 when the index is out of range or on error.
 */
int32_t kmsg_read(uint32_t index, kmsg_entry_t* entry);

#endif // _LIBSYS_KMSG_H
