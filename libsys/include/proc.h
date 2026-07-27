#ifndef _LIBSYS_PROC_H
#define _LIBSYS_PROC_H

#include <stdint.h>

/**
 * Exits the current process with the given status code.
 *
 * @param status The status code to exit with
 */
void _exit(int status);

/**
 * Spawns a child process from an executable and waits for it to finish.
 *
 * Blocks until the child exits and returns its exit code.
 *
 * @param path The path to the executable
 * @param argv NULL terminated argument vector (argv[0] is conventionally the path)
 * @return The child's exit code, or -1 if the child could not be created
 */
int spawn(const char* path, char* const argv[]);

#endif // _LIBSYS_PROC_H
