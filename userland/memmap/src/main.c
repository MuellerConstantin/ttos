#include <memmap.h>
#include <stdio.h>
#include <termio.h>

/** Room for the widest line a region can produce. */
#define MEMMAP_LINE_LENGTH 128

int main(void) {
    memregion_t region;
    termio_pager_t pager;
    char line[MEMMAP_LINE_LENGTH];

    termio_pager_init(&pager);

    for (uint32_t index = 0; memmap_read(index, &region) == 0; index++) {
        sprintf(line, "Memory region: %x - %x (%d bytes) Type: %x\n",
                region.base, region.base + region.length - 1, region.length, region.type);

        // The reader has seen enough, the rest of the map is not worth printing.
        if (termio_pager_puts(&pager, line) < 0) {
            break;
        }
    }

    return 0;
}
