#include <memmap.h>
#include <stdio.h>

int main(void) {
    memregion_t region;

    for (uint32_t index = 0; memmap_read(index, &region) == 0; index++) {
        printf("Memory region: %x - %x (%d bytes) Type: %x\n",
               region.base, region.base + region.length - 1, region.length, region.type);
    }

    return 0;
}
