#include <stdio.h>
#include <sysinfo.h>

void _start() {
    osinfo_t osinfo;
    meminfo_t meminfo;

    puts("OS Information: ");

    if (sysinfo_get_osinfo(&osinfo) < 0) {
        puts("Failed to get system information\n");
    } else {
        printf("%s %s %s %s\n", osinfo.name, osinfo.version, osinfo.platform, osinfo.arch);
    }

    puts("Memory Information: ");

    if (sysinfo_get_meminfo(&meminfo) < 0) {
        puts("Failed to get memory information\n");
    } else {
        double total_memory_mb = meminfo.total / 1024 / 1024;
        double free_memory_mb = meminfo.free / 1024 / 1024;
        double used_memory_mb = total_memory_mb - free_memory_mb;
        double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

        printf("%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);
    }

    while(1);
}
