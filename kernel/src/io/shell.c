#include <io/shell.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>

static void shell_process_command(shell_t *shell, const char *command);

shell_t* shell_create(tty_t *tty0) {
    shell_t *shell = kmalloc(sizeof(shell_t));

    if(shell == NULL) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, KPANIC_KHEAP_OUT_OF_MEMORY_CODE, NULL);
    }

    shell->tty = tty0;

    return shell;
}

void shell_execute(shell_t *shell) {
    while(1) {
        tty_putchar(shell->tty, '>');

        char *command = tty_readline(shell->tty, true);

        shell_process_command(shell, command);

        kfree(command);
    }
}

static void shell_process_command(shell_t *shell, const char *command) {
}
