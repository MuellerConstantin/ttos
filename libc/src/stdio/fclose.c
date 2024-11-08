#include <stdio.h>

int fclose(FILE* stream) {
    if(stream == NULL) {
        return EOF;
    }

    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x03, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(stream->fd)
        : "%eax", "%ebx"
    );

    if(return_value < 0) {
        return EOF;
    }

    return 0;
}
