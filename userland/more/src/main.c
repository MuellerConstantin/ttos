#include <fsio.h>
#include <stdio.h>
#include <termio.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: more <path>\n");
        return 1;
    }

    int32_t fd = fsio_open(argv[1], FSIO_RDONLY, 0);

    if (fd < 0) {
        puts("more: cannot open file\n");
        return 1;
    }

    termio_pager_t pager;

    termio_pager_init(&pager);

    char buffer[512];
    int32_t bytes_read;

    while (!termio_pager_stopped(&pager) && (bytes_read = fsio_read(fd, buffer, sizeof(buffer))) > 0) {
        for (int32_t i = 0; i < bytes_read; i++) {
            if (termio_pager_putchar(&pager, buffer[i]) < 0) {
                break;
            }
        }
    }

    fsio_close(fd);

    return 0;
}
