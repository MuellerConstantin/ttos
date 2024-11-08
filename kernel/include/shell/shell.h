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

/**
 * Returns to shell execution after user program has exited.
 * 
 * @param exit_code The exit code, only valid if the program exited normally and the exception code is -1.
 * @param exception_code The exception code if the program exited due to an exception, otherwise -1.
 */
void shell_revert(int32_t exit_code, int32_t exception_code);

#endif // _KERNEL_SHELL_H
