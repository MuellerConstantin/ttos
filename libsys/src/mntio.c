#include <mntio.h>

int32_t mntio_list(uint32_t index, mntinfo_t* info) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov $0x12, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(index), "g"(info)
        : "%eax", "%ebx", "%ecx"
    );

    return return_value;
}

int32_t mntio_mount(char drive, const char* id) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov $0x13, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"((int32_t) drive), "g"(id)
        : "%eax", "%ebx", "%ecx"
    );

    return return_value;
}

int32_t mntio_unmount(char drive) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x14, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"((int32_t) drive)
        : "%eax", "%ebx"
    );

    return return_value;
}
