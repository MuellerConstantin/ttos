#include <volio.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    volinfo_t info;
    char size[16];

    /*
     * The name is the only column of unpredictable width, so it goes last and
     * nothing can push the columns out of line.
     */
    printf("%-8s%6s  %s\n", "ID", "SIZE", "NAME");

    for (uint32_t index = 0; volio_list(index, &info) == 0; index++) {
        sizetoa(info.size, size);

        printf("%-8s%6s  %s\n", info.id, size, info.name);
    }

    return 0;
}
