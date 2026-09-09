#include <fsio.h>
#include <stdio.h>

int main(int argc, char** argv) {
    // Exactly one operand, and no options: unlink is the bare call, not a convenience front end.
    if (argc != 2) {
        puts("usage: unlink <path>\n");
        return 1;
    }

    if (fsio_unlink(argv[1]) != 0) {
        puts("unlink: cannot remove file\n");
        return 1;
    }

    return 0;
}
