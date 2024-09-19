#ifndef _KERNEL_IO_SHELL_H
#define _KERNEL_IO_SHELL_H

#include <stddef.h>
#include <io/stream.h>

typedef struct shell shell_t;

struct shell {
    void (*stdout)(char);
    char (*stdin)(void);
};

/**
 * Creates a new shell.
 * 
 * @param stdout The function to write a character to the shell.
 * @param stdin The function to read a character from the shell.
 * @return The shell.
 */
shell_t* shell_create(void (stdout)(char), char (stdin)(void));

/**
 * Executes the shell.
 * 
 * @param shell The shell to execute.
 */
void shell_execute(shell_t *shell);

#endif // _KERNEL_IO_SHELL_H
