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
