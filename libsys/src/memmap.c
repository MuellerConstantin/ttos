#include <memmap.h>

int32_t memmap_read(uint32_t index, memregion_t* region) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov $0x17, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(index), "g"(region)
        : "%eax", "%ebx", "%ecx"
    );

    return return_value;
}
