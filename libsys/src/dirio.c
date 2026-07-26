#include <dirio.h>

int32_t dirio_open(const char* path) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x0C, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(path)
        : "%eax", "%ebx"
    );

    return return_value;
}

int32_t dirio_read(int32_t dd, dirent_t* entry) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov $0x0D, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(dd), "g"(entry)
        : "%eax", "%ebx", "%ecx"
    );

    return return_value;
}

int32_t dirio_close(int32_t dd) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x0E, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(dd)
        : "%eax", "%ebx"
    );

    return return_value;
}
