#include <fsio.h>

int32_t fsio_read(int32_t fd, void* buffer, size_t size) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov %3, %%edx\n"
        "mov $0x00, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(fd), "g"(buffer), "g"(size)
        : "%eax", "%ebx", "%ecx", "%edx"
    );

    return return_value;
}

int32_t fsio_write(int32_t fd, const void* buffer, size_t size) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov %3, %%edx\n"
        "mov $0x01, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(fd), "g"(buffer), "g"(size)
        : "%eax", "%ebx", "%ecx", "%edx"
    );

    return return_value;
}

int32_t fsio_open(const char* path, int32_t flags, int32_t mode) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov %3, %%edx\n"
        "mov $0x02, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(path), "g"(flags), "g"(mode)
        : "%eax", "%ebx", "%ecx", "%edx"
    );

    return return_value;
}

int32_t fsio_close(int32_t fd) {
    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov $0x03, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "r"(fd)
        : "%eax", "%ebx"
    );

    return return_value;
}
