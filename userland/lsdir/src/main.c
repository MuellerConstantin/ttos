#include <dirio.h>
#include <stdio.h>
#include <termio.h>

int main(int argc, char** argv) {
    if (argc > 2) {
        puts("usage: lsdir [<path>]\n");
        return 1;
    }

    // Without a path the working directory is listed; the kernel resolves "." against it.
    const char* path = argc == 2 ? argv[1] : ".";

    int32_t dd = dirio_open(path);

    if (dd < 0) {
        puts("lsdir: cannot open directory\n");
        return 1;
    }

    dirent_t entry;
    termio_pager_t pager;

    termio_pager_init(&pager);

    while (dirio_read(dd, &entry) == 0) {
        // The reader has seen enough, the rest of the directory is not worth reading.
        if (termio_pager_puts(&pager, entry.name) < 0 || termio_pager_putchar(&pager, '\n') < 0) {
            break;
        }
    }

    dirio_close(dd);

    return 0;
}
