#ifndef _LIBSYS_PROC_H
#define _LIBSYS_PROC_H

#include <stdint.h>
#include <stddef.h>
#include <ttos/syscall.h>

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
 * Spawns a child process from an executable. The child runs alongside the
 * caller and receives a copy of the caller's environment; its outcome is
 * collected with wait.
 *
 * @param path The path to the executable
 * @param argv NULL terminated argument vector (argv[0] is conventionally the path)
 * @param flags 0, or SPAWN_FOREGROUND to hand the child the terminal's
 *              foreground as it is created (see termio_set_foreground); only
 *              the current foreground process may do that
 * @return The PID of the child, or a negative value if the executable could
 *         not be started or the foreground could not be handed over.
 */
pid_t spawn(const char* path, char* const argv[], int flags);

/**
 * Collects a child that has exited. Blocks until one does, unless WAIT_NOHANG
 * is given.
 *
 * @param pid The PID of the child to wait for, or -1 for any child
 * @param status Receives the child's status if not NULL: its exit code, or
 *               128 + the exception number if it was terminated by a fault
 * @param options 0, or WAIT_NOHANG to return instead of blocking
 * @return The PID of the collected child, 0 if WAIT_NOHANG was given and no
 *         child has exited yet, or -1 if the caller has no such child.
 */
pid_t wait(pid_t pid, int* status, int options);

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
