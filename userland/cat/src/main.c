#include <fsio.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: cat <path>\n");
        return 1;
    }

    int32_t fd = fsio_open(argv[1], FSIO_RDONLY, 0);

    if (fd < 0) {
        puts("cat: cannot open file\n");
        return 1;
    }

    char buffer[512];
    int32_t bytes_read;

    while ((bytes_read = fsio_read(fd, buffer, sizeof(buffer))) > 0) {
        fsio_write(FSIO_STDOUT, buffer, bytes_read);
    }

    fsio_close(fd);

    return 0;
}
