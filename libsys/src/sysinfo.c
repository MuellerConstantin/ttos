#include <sysinfo.h>

int32_t sysinfo_get_osinfo(osinfo_t* info) {
    uint32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x04, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(info)
        : "%eax", "%ebx"
    );

    if(return_value < 0) {
        return -1;
    }

    return return_value;
}

int32_t sysinfo_get_meminfo(meminfo_t* info) {
    uint32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x05, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(info)
        : "%eax", "%ebx"
    );

    if(return_value < 0) {
        return -1;
    }

    return return_value;
}
