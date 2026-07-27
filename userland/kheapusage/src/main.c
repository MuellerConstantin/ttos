#include <sysinfo.h>
#include <stdio.h>

int main(void) {
    meminfo_t kheapinfo;

    if (sysinfo_get_kheapinfo(&kheapinfo) < 0) {
        puts("kheapusage: failed to get kernel heap information\n");
        return 1;
    }

    double total_memory_mb = kheapinfo.total / 1024 / 1024;
    double free_memory_mb = kheapinfo.free / 1024 / 1024;
    double used_memory_mb = total_memory_mb - free_memory_mb;
    double used_memory_percentage = (used_memory_mb / total_memory_mb) * 100;

    printf("%f MB / %f MB (%f%%) used\n", used_memory_mb, total_memory_mb, used_memory_percentage);

    return 0;
}
