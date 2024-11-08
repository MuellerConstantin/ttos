#ifndef _KERNEL_SHELL_H
#define _KERNEL_SHELL_H

#include <stddef.h>
#include <io/stream.h>

/**
 * Initializes the shell.
 * 
 * @param out The output stream.
 * @param in The input stream.
 * @param err The error stream.
 */
void shell_init(stream_t* out, stream_t* in, stream_t* err);

/**
 * Executes the shell.
 * 
 * @param shell The shell to execute.
 */
void shell_execute();

#endif // _KERNEL_SHELL_H
