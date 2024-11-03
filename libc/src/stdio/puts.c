#include <stdio.h>

int puts(const char* str) {
    /*
     * Naive implementation of puts that writes a string to the standard output stream by
     * using the syscall interface directly.
     */

    __asm__ volatile(
        "mov %0, %%ebx\n"
        "mov $0x00, %%eax\n"
        "int $0x80\n"
        :
        : "r"(str)
    );
}
