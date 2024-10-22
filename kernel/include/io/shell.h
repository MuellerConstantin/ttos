#ifndef _KERNEL_IO_SHELL_H
#define _KERNEL_IO_SHELL_H

#include <stddef.h>
#include <io/stream.h>

typedef struct shell shell_t;

struct shell {
    stream_t* out;
    stream_t* in;
    stream_t* err;
};

/**
 * Creates a new shell.
 * 
 * @param out The output stream.
 * @param in The input stream.
 * @param err The error stream.
 * @return The shell.
 */
shell_t* shell_create(stream_t* out, stream_t* in, stream_t* err);

/**
 * Executes the shell.
 * 
 * @param shell The shell to execute.
 */
void shell_execute(shell_t *shell);

#endif // _KERNEL_IO_SHELL_H
