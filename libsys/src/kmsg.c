#include <kmsg.h>

int32_t kmsg_read(uint32_t index, kmsg_entry_t* entry) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov $0x15, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(index), "g"(entry)
        : "%eax", "%ebx", "%ecx"
    );

    return return_value;
}
