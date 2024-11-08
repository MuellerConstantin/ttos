#include <hmap.h>

void* hmap_alloc(size_t n_pages) {
    void* return_value = NULL;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x0A, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(n_pages)
        : "%eax", "%ebx"
    );

    return return_value;
}
