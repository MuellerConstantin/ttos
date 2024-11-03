#include <stdio.h>
#include <string.h>

int puts(const char* str) {
    /*
     * Naive implementation of puts that writes a string to the standard output stream by
     * using the syscall interface directly.
     */

    uint32_t return_value = 0;

    __asm__ volatile(
        "mov %0, %%ebx\n"
        "mov $0x00, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(str)
    );

    if(return_value == 0) {
        return strlen(str);
    } else {
        return -1;
    }
}
