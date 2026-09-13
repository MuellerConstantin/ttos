#ifndef _LIBSYS_VOLIO_H
#define _LIBSYS_VOLIO_H

#include <stdint.h>
#include <stddef.h>
#include <ttos/syscall.h>

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

/**
 * Reads from a volume, past any file system on it. The offset is counted from
 * the start of the volume, not of the device it sits on.
 *
 * @param id The short id of the volume, as listed in volinfo.
 * @param offset The offset into the volume in bytes.
 * @param buffer The buffer to read into.
 * @param size The number of bytes to read.
 * @return The number of bytes read, which is short at the end of the volume, or -1 if it is unknown.
 */
int32_t volio_read(const char* id, size_t offset, void* buffer, size_t size);

/**
 * Writes to a volume, past any file system on it. This is how a file system is
 * created. Writes are cut short at the end of the volume, so what is written
 * cannot land on a neighbouring partition or the partition table.
 *
 * @param id The short id of the volume, as listed in volinfo.
 * @param offset The offset into the volume in bytes.
 * @param buffer The buffer to write from.
 * @param size The number of bytes to write.
 * @return The number of bytes written, which is short at the end of the volume, or -1 if it is unknown.
 */
int32_t volio_write(const char* id, size_t offset, const void* buffer, size_t size);

#endif // _LIBSYS_VOLIO_H
