#include <sysinfo.h>
#include <stdio.h>

int main(void) {
    uint32_t seconds = sysinfo_get_uptime();

    uint32_t weeks = seconds / (60 * 60 * 24 * 7);
    seconds %= (60 * 60 * 24 * 7);

    uint32_t days = seconds / (60 * 60 * 24);
    seconds %= (60 * 60 * 24);

    uint32_t hours = seconds / (60 * 60);
    seconds %= (60 * 60);

    uint32_t minutes = seconds / 60;
    seconds %= 60;

    printf("up ");

    if (weeks > 0) {
        printf("%d weeks, ", weeks);
    }

    if (days > 0) {
        printf("%d days, ", days);
    }

    if (hours > 0) {
        printf("%d hours, ", hours);
    }

    if (minutes > 0) {
        printf("%d minutes, ", minutes);
    }

    printf("%d seconds\n", seconds);

    return 0;
}
