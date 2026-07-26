#include <power.h>

int32_t power_off(void) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov $0x10, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        :
        : "%eax"
    );

    return return_value;
}
