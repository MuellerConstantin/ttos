#include <sysinfo.h>
#include <syscall.h>

int32_t sysinfo_get_osinfo(osinfo_t* info) {
    return syscall1(SYSCALL_GET_OSINFO, (uint32_t) info);
}

int32_t sysinfo_get_meminfo(meminfo_t* info) {
    return syscall1(SYSCALL_GET_MEMINFO, (uint32_t) info);
}

int32_t sysinfo_get_kheapinfo(meminfo_t* info) {
    return syscall1(SYSCALL_GET_KHEAPINFO, (uint32_t) info);
}

uint32_t sysinfo_get_uptime(void) {
    return (uint32_t) syscall0(SYSCALL_UPTIME);
}

int32_t sysinfo_get_terminfo(terminfo_t* info) {
    return syscall1(SYSCALL_GET_TERMINFO, (uint32_t) info);
}
