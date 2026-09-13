#include <mntio.h>
#include <stdio.h>
#include <stdlib.h>

/** Room for the widest line the columns below can produce. */
#define VOLUSAGE_LINE_LENGTH 128

int main(void) {
    mntinfo_t mount;
    fsinfo_t usage;
    char line[VOLUSAGE_LINE_LENGTH];
    char total[16];
    char used[16];
    char free_space[16];

    printf("%-7s%-8s%8s%8s%8s%9s\n", "DRIVE", "VOLUME", "SIZE", "USED", "FREE", "USE%");

    for (uint32_t index = 0; mntio_list(index, &mount) == 0; index++) {
        if (mntio_usage(mount.drive, &usage) != 0) {
            sprintf(line, "%c:     %-8s%8s%8s%8s%9s\n", mount.drive, mount.volume_id, "-", "-", "-", "-");
            puts(line);
            continue;
        }

        sizetoa(usage.total, total);
        sizetoa(usage.total - usage.free, used);
        sizetoa(usage.free, free_space);

        // An empty file system would divide by zero, and there is nothing to report about it either.
        double percentage = usage.total > 0 ? ((double) (usage.total - usage.free) / (double) usage.total) * 100 : 0;

        sprintf(line, "%c:     %-8s%8s%8s%8s%8f%%\n", mount.drive, mount.volume_id, total, used, free_space, percentage);

        puts(line);
    }

    return 0;
}
