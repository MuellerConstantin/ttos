#ifndef _KERNEL_IO_SHELL_H
#define _KERNEL_IO_SHELL_H

#include <stddef.h>
#include <io/tty.h>

typedef struct shell shell_t;

struct shell {
    tty_t *tty;
};

/**
 * Creates a new shell.
 * 
 * @param tty The TTY to use.
 * @return The shell.
 */
shell_t* shell_create(tty_t *tty);

/**
 * Executes the shell.
 * 
 * @param shell The shell to execute.
 */
void shell_execute(shell_t *shell);

#endif // _KERNEL_IO_SHELL_H
