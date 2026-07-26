#include <dirio.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: lsdir <path>\n");
        return 1;
    }

    int32_t dd = dirio_open(argv[1]);

    if (dd < 0) {
        puts("lsdir: cannot open directory\n");
        return 1;
    }

    dirent_t entry;

    while (dirio_read(dd, &entry) == 0) {
        puts(entry.name);
        putchar('\n');
    }

    dirio_close(dd);

    return 0;
}
