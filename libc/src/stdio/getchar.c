#include <stdio.h>

int getchar(void) {
    char ch;
    uint32_t return_value = 0;

    do {
        __asm__ volatile(
            "mov $0x00, %%ebx\n"
            "mov %1, %%ecx\n"
            "mov %2, %%edx\n"
            "mov $0x00, %%eax\n"
            "int $0x80\n"
            "mov %%eax, %0\n"
            : "=r"(return_value)
            : "r"(&ch), "r"(sizeof(ch))
            : "%eax", "%ebx", "%ecx", "%edx"
        );

        if(return_value == -1) {
            return EOF;
        }
    } while(return_value == 0);

    return ch;
}
