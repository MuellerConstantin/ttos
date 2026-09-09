#include <fsio.h>
#include <stdio.h>

#define MKDIR_PERMISSIONS 0755

int main(int argc, char** argv) {
    if (argc < 2) {
        puts("usage: mkdir <path>...\n");
        return 1;
    }

    int status = 0;

    // Carry on after a failure, so one bad operand does not swallow the rest.
    for (int index = 1; index < argc; index++) {
        if (fsio_mkdir(argv[index], MKDIR_PERMISSIONS) != 0) {
            printf("mkdir: cannot create directory: %s\n", argv[index]);
            status = 1;
        }
    }

    return status;
}
