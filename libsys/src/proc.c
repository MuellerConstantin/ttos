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

int spawn(const char* path, char* const argv[]) {
    int result;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov $0x19, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(result)
        : "r"(path), "r"(argv)
        : "%eax", "%ebx", "%ecx", "memory"
    );

    return result;
}
