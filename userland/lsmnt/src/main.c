#include <mntio.h>
#include <stdio.h>

int main(void) {
    mntinfo_t info;

    for (uint32_t index = 0; mntio_list(index, &info) == 0; index++) {
        printf("%c:\n", info.drive);
    }

    return 0;
}
