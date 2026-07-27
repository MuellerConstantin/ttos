#include <mntio.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: unmount <drive>\n");
        return 1;
    }

    char drive = argv[1][0];

    // Accept a lowercase drive letter for convenience.
    if (drive >= 'a' && drive <= 'z') {
        drive -= 'a' - 'A';
    }

    switch (mntio_unmount(drive)) {
        case UNMOUNT_OK:
            return 0;
        case UNMOUNT_ERR_NOT_MOUNTED:
            puts("unmount: drive not mounted\n");
            return 1;
        default:
            puts("unmount: failed to unmount\n");
            return 1;
    }
}
