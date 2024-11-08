#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* fopen(const char* filename, const char* mode) {
    uint32_t flags;

    if (strcmp(mode, "r") == 0) {
        flags = O_RDONLY;
    } else if (strcmp(mode, "r+") == 0) {
        flags = O_RDWR;
    } else if (strcmp(mode, "w") == 0) {
        flags = O_WRONLY | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "w+") == 0) {
        flags = O_RDWR | O_CREAT | O_TRUNC;
    } else if (strcmp(mode, "a") == 0) {
        flags = O_WRONLY | O_CREAT | O_APPEND;
    } else if (strcmp(mode, "a+") == 0) {
        flags = O_RDWR | O_CREAT | O_APPEND;
    } else {
        return NULL;
    }

    int32_t return_value = 0;

    __asm__ volatile(
        "mov %1, %%ebx\n"
        "mov %2, %%ecx\n"
        "mov %3, %%edx\n"
        "mov $0x02, %%eax\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(return_value)
        : "g"(filename), "g"(flags), "g"(0644)
        : "%eax", "%ebx", "%ecx", "%edx"
    );

    if(return_value == -1) {
        return NULL;
    }

    FILE* file = malloc(sizeof(FILE));

    if(file == NULL) {
        return NULL;
    }

    file->fd = return_value;
    file->flags = flags;

    return file;
}
