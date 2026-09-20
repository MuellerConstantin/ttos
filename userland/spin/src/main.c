#include <stdio.h>
#include <sysinfo.h>

/*
 * Busy-waits for a number of seconds, printing a dot per second. There is no
 * sleep; time is taken from the uptime, so the process keeps the CPU the whole
 * time. Meant to be run in the background to watch processes run side by side.
 */
int main(int argc, char** argv) {
    uint32_t seconds = 5;

    if(argc > 1) {
        seconds = 0;

        for(const char* cursor = argv[1]; *cursor != '\0'; cursor++) {
            if(*cursor < '0' || *cursor > '9') {
                printf("usage: spin [seconds]\n");
                return 1;
            }

            seconds = seconds * 10 + (*cursor - '0');
        }
    }

    uint32_t start = sysinfo_get_uptime();
    uint32_t last = start;

    while(sysinfo_get_uptime() - start < seconds) {
        uint32_t now = sysinfo_get_uptime();

        if(now != last) {
            putchar('.');
            last = now;
        }
    }

    putchar('\n');

    return 0;
}
