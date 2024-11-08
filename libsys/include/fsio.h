#ifndef _LIBSYS_FSIO_H
#define _LIBSYS_FSIO_H

#include <stdint.h>
#include <stddef.h>

#define FSIO_STDIN 0
#define FSIO_STDOUT 1
#define FSIO_STDERR 2

#define FSIO_RDONLY    0b00000001
#define FSIO_WRONLY    0b00000010
#define FSIO_RDWR      (FSIO_RDONLY | FSIO_WRONLY)
#define FSIO_CREAT     0b00000100
#define FSIO_TRUNC     0b00001000
#define FSIO_APPEND    0x00010000

/**
 * Reads from a file descriptor.
 * 
 * @param fd The file descriptor to read from.
 * @param buffer The buffer to read into.
 * @param size The number of bytes to read.
 * @return The number of bytes read or -1 on error.
 */
int32_t fsio_read(int32_t fd, void* buffer, size_t size);

/**
 * Writes to a file descriptor.
 * 
 * @param fd The file descriptor to write to.
 * @param buffer The buffer to write from.
 * @param size The number of bytes to write.
 * @return The number of bytes written or -1 on error.
 */
int32_t fsio_write(int32_t fd, const void* buffer, size_t size);

/**
 * Opens a file.
 * 
 * @param path The path to the file.
 * @param flags The flags to open the file with.
 * @param mode The mode to open the file with.
 * @return The file descriptor or -1 on error.
 */
int32_t fsio_open(const char* path, int32_t flags, int32_t mode);

/**
 * Closes a file descriptor.
 * 
 * @param fd The file descriptor to close.
 * @return 0 on success or -1 on error.
 */
int32_t fsio_close(int32_t fd);

#endif // _LIBSYS_FSIO_H
