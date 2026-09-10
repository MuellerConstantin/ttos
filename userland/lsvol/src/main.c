#include <volio.h>
#include <stdio.h>
#include <stdlib.h>
#include <termio.h>

/** Room for the widest line the columns below can produce. */
#define LSVOL_LINE_LENGTH 128

int main(void) {
    volinfo_t info;
    termio_pager_t pager;
    char size[16];
    char line[LSVOL_LINE_LENGTH];

    termio_pager_init(&pager);

    /*
     * The name is the only column of unpredictable width, so it goes last and
     * nothing can push the columns out of line.
     */
    sprintf(line, "%-8s%6s  %s\n", "ID", "SIZE", "NAME");
    termio_pager_puts(&pager, line);

    for (uint32_t index = 0; volio_list(index, &info) == 0; index++) {
        sizetoa(info.size, size);

        sprintf(line, "%-8s%6s  %s\n", info.id, size, info.name);

        // The reader has seen enough, the rest of the list is not worth listing.
        if (termio_pager_puts(&pager, line) < 0) {
            break;
        }
    }

    return 0;
}
