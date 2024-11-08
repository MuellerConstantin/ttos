#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fsio.h>

FILE* fopen(const char* filename, const char* mode) {
    uint32_t flags;

    if (strcmp(mode, "r") == 0) {
        flags = FSIO_RDONLY;
    } else if (strcmp(mode, "r+") == 0) {
        flags = FSIO_RDWR;
    } else if (strcmp(mode, "w") == 0) {
        flags = FSIO_WRONLY | FSIO_CREAT | FSIO_TRUNC;
    } else if (strcmp(mode, "w+") == 0) {
        flags = FSIO_RDWR | FSIO_CREAT | FSIO_TRUNC;
    } else if (strcmp(mode, "a") == 0) {
        flags = FSIO_WRONLY | FSIO_CREAT | FSIO_APPEND;
    } else if (strcmp(mode, "a+") == 0) {
        flags = FSIO_RDWR | FSIO_CREAT | FSIO_APPEND;
    } else {
        return NULL;
    }

    int32_t fd = fsio_open(filename, flags, 0666);

    if (fd < 0) {
        return NULL;
    }

    FILE* file = malloc(sizeof(FILE));

    if(file == NULL) {
        return NULL;
    }

    file->fd = fd;
    file->flags = flags;

    return file;
}
