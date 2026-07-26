#ifndef _LIBSYS_DIRIO_H
#define _LIBSYS_DIRIO_H

#include <stdint.h>
#include <stddef.h>

typedef struct dirent dirent_t;

struct dirent {
    char name[256];
    uint32_t inode;
};

/**
 * Opens a directory for reading.
 *
 * @param path The absolute path to the directory.
 * @return The directory descriptor or -1 on error.
 */
int32_t dirio_open(const char* path);

/**
 * Reads the next entry from a directory.
 *
 * @param dd The directory descriptor to read from.
 * @param entry The entry to fill.
 * @return 0 on success or -1 when there are no more entries or on error.
 */
int32_t dirio_read(int32_t dd, dirent_t* entry);

/**
 * Closes a directory descriptor.
 *
 * @param dd The directory descriptor to close.
 * @return 0 on success or -1 on error.
 */
int32_t dirio_close(int32_t dd);

#endif // _LIBSYS_DIRIO_H
