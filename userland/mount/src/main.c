#include <mntio.h>
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 3) {
        puts("usage: mount <drive> <id>\n");
        return 1;
    }

    char drive = argv[1][0];

    // Accept a lowercase drive letter for convenience.
    if (drive >= 'a' && drive <= 'z') {
        drive -= 'a' - 'A';
    }

    switch (mntio_mount(drive, argv[2])) {
        case MOUNT_OK:
            return 0;
        case MOUNT_ERR_NOT_FOUND:
            puts("mount: volume not found\n");
            return 1;
        case MOUNT_ERR_IN_USE:
            puts("mount: drive already mounted\n");
            return 1;
        default:
            puts("mount: failed to mount, filesystem may not be supported\n");
            return 1;
    }
}
