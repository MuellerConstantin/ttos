/**
 * @file path.h
 * @brief Path resolution against a working directory.
 *
 * Every path that reaches the kernel from userland may be relative. The file and directory layers
 * only accept absolute paths of the form X:/..., so a path is resolved against the working directory
 * of the calling process first and normalized on the way: '.' and '..' are folded lexically, so a
 * working directory never carries them and '..' works on file systems without such entries.
 */

#ifndef _KERNEL_IO_PATH_H
#define _KERNEL_IO_PATH_H

#include <stdint.h>
#include <stdbool.h>
#include <ttos/syscall.h>

/**
 * Resolve a path against a working directory.
 *
 * Accepted forms are X:/..., which is taken as is, /... which is placed on the drive of the
 * working directory, and anything else, which is appended to the working directory. A drive
 * relative path such as X:foo is refused, there is no working directory per drive.
 *
 * @param cwd The working directory, absolute and normalized.
 * @param path The path to resolve.
 * @param resolved Buffer of at least PATH_MAX bytes for the absolute, normalized result. The
 *                 drive letter is upper case and the root of a drive is written as X:/, any
 *                 other directory without a trailing slash.
 * @return 0 on success or -1 if the path is malformed or does not fit.
 */
int32_t path_resolve(const char* cwd, const char* path, char* resolved);

#endif // _KERNEL_IO_PATH_H
