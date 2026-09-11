#include <mntio.h>
#include <stdio.h>

int main(void) {
    mntinfo_t info;

    printf("%-7s%-8s%s\n", "DRIVE", "VOLUME", "TYPE");

    for (uint32_t index = 0; mntio_list(index, &info) == 0; index++) {
        printf("%c:     %-8s%s\n", info.drive, info.volume_id, info.fs_type);
    }

    return 0;
}
