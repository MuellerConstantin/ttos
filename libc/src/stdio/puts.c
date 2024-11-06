#include <stdio.h>
#include <string.h>

int puts(const char* str) {
    /*
     * Naive implementation of puts that writes a string to the standard output stream by
     * using the syscall interface directly.
     */

    uint32_t return_value = 0;
    size_t length = strlen(str);

    __asm__ volatile(
        "mov $0x01, %%ebx\n"
        "mov %1, %%ecx\n"
        "mov %2, %%edx\n"
        "mov $0x01, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(str), "r"(length)
        : "%eax", "%ebx", "%ecx", "%edx"
    );

    if(return_value < 0) {
        return EOF;
    }

    return length;
}
