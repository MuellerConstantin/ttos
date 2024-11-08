#include <stdio.h>

size_t fread(void* buffer, size_t size, size_t count, FILE* stream) {
    if(buffer == NULL || stream == NULL || size == 0 || count == 0) {
        return 0;
    }

    size_t total_bytes = size * count;
    size_t bytes_read = 0;

    while(bytes_read < total_bytes) {
        size_t bytes_to_read = total_bytes - bytes_read;
        int32_t return_value = 0;

        __asm__ volatile(
            "mov %1, %%ebx\n"
            "mov %2, %%ecx\n"
            "mov %3, %%edx\n"
            "mov $0x00, %%eax\n"
            "int $0x80\n"
            "mov %%eax, %0\n"
            : "=r"(return_value)
            : "g"(stream->fd), "g"((char *) buffer + bytes_read), "g"(total_bytes - bytes_read)
            : "%eax", "%ebx", "%ecx", "%edx"
        );

        if(return_value <= 0) {
            break;
        }

        bytes_read += return_value;
    }

    return bytes_read / size;
}
