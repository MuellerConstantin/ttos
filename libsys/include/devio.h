#ifndef _LIBSYS_DEVIO_H
#define _LIBSYS_DEVIO_H

#include <stdint.h>
#include <stddef.h>
#include <ttos/syscall.h>

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

/**
 * Queries the medium behind a storage device.
 *
 * @param id The short id of the device, as listed in devinfo.
 * @param info The storage information to fill.
 * @return 0 on success or -1 if the device is unknown or carries no storage.
 */
int32_t devio_get_storageinfo(const char* id, storageinfo_t* info);

/**
 * Reads from a storage device, past any file system on it.
 *
 * @param id The short id of the device, as listed in devinfo.
 * @param offset The offset into the device in bytes.
 * @param buffer The buffer to read into.
 * @param size The number of bytes to read.
 * @return The number of bytes read or -1 if the device is unknown or carries no storage.
 */
int32_t devio_read(const char* id, size_t offset, void* buffer, size_t size);

/**
 * Writes to a storage device, past any file system on it. There is nothing
 * between this and the medium: a partition table, a boot sector and a file
 * system that is being created are all written this way, and so is anything
 * that destroys them.
 *
 * @param id The short id of the device, as listed in devinfo.
 * @param offset The offset into the device in bytes.
 * @param buffer The buffer to write from.
 * @param size The number of bytes to write.
 * @return The number of bytes written or -1 if the device is unknown or carries no storage.
 */
int32_t devio_write(const char* id, size_t offset, const void* buffer, size_t size);

/**
 * Scans a device for volumes again, after its partition table was changed.
 * Volumes found before are discarded, so their ids do not survive the call.
 *
 * @param id The short id of the device, as listed in devinfo.
 * @return The number of volumes found, -1 if the device is unknown or carries
 *         no storage, or -2 while one of its volumes is still mounted.
 */
int32_t devio_rescan(const char* id);

#endif // _LIBSYS_DEVIO_H
