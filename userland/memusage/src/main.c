#include <sysinfo.h>
#include <stdio.h>

int main(void) {
    meminfo_t meminfo;

    if (sysinfo_get_meminfo(&meminfo) < 0) {
        puts("memusage: failed to get memory information\n");
        return 1;
    }

    double total_memory_mb = meminfo.total / 1024 / 1024;
    double free_memory_mb = meminfo.free / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    printf("%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);

    return 0;
}
