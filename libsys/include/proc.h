#ifndef _LIBSYS_PROC_H
#define _LIBSYS_PROC_H

#include <stdint.h>
#include <stddef.h>

/**
 * The environment of the current process, a NULL terminated array of strings of the form
 * NAME=VALUE. Set by the startup code to the array the kernel placed on the initial stack;
 * a process that changes its environment may point it at an array of its own.
 */
extern char** environ;

/**
 * Exits the current process with the given status code.
 *
 * @param status The status code to exit with
 */
void _exit(int status);

/**
 * Spawns a child process from an executable and waits for it to finish.
 *
 * Blocks until the child exits and returns its exit code. The child receives a
 * copy of the caller's environment.
 *
 * @param path The path to the executable
 * @param argv NULL terminated argument vector (argv[0] is conventionally the path)
 * @return A non-negative status if the program ran (its exit code, or
 *         128 + the exception number if it was terminated by a fault), or a
 *         negative value if the executable could not be started.
 */
int spawn(const char* path, char* const argv[]);

/**
 * Changes the working directory of the current process. Relative paths the
 * process hands to the kernel from then on are resolved against it, and a
 * child spawned afterwards starts there.
 *
 * @param path The directory, absolute or relative to the current working directory
 * @return 0 on success or -1 if the path is malformed, does not exist or is no directory
 */
int chdir(const char* path);

/**
 * Copies the working directory of the current process into a buffer, as an
 * absolute, normalized path (see PATH_MAX for the longest possible).
 *
 * @param buffer The buffer to fill
 * @param size The size of the buffer in bytes
 * @return 0 on success or -1 if the buffer is too small
 */
int getcwd(char* buffer, size_t size);

#endif // _LIBSYS_PROC_H
