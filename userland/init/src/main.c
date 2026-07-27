#include <stdio.h>
#include <proc.h>

int main(void) {
    const char* shell_path = "A:/shell.elf";
    char* shell_argv[] = { (char*) shell_path, 0 };

    /*
     * init is PID 1: it must never exit. Keep a shell running and respawn it if
     * it ever terminates. If init itself were to return, the kernel raises a
     * panic (see process_terminate).
     */
    for(;;) {
        int code = spawn(shell_path, shell_argv);

        printf("init: shell exited (%d), restarting\n", code);
    }

    return 0;
}
