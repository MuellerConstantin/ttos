#include <proc.h>

void _exit(int status) {
    __asm__ volatile(
        "mov %0, %%ebx\n"
        "mov $0x0B, %%eax\n"
        "int $0x80\n"
        :
        : "r"(status)
        : "%eax", "%ebx"
    );
}
