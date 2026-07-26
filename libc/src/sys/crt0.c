#include <proc.h>

extern int main(int argc, char** argv, char** envp);

/*
 * Program entry point. The kernel prepares the initial user stack so that argc sits at the very
 * top, directly followed by the argv pointer array (argv[0] .. argv[argc - 1], NULL). This stub
 * reads them off the stack, calls main, and exits with its return value.
 *
 * It has to be a naked function so the compiler emits no prologue that would shift esp before we
 * can read the layout the kernel set up.
 */
__attribute__((naked, noreturn)) void _start(void) {
    __asm__(
        "mov (%esp), %eax\n"    // argc
        "lea 4(%esp), %ebx\n"   // argv (array follows argc on the stack)
        "push $0\n"             // envp = NULL
        "push %ebx\n"           // argv
        "push %eax\n"           // argc
        "call main\n"
        "push %eax\n"           // exit status = main's return value
        "call _exit\n"
    );
}
