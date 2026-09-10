#include <proc.h>
#include <syscall.h>

void _exit(int status) {
    syscall1(SYSCALL_EXIT, (uint32_t) status);
}

int spawn(const char* path, char* const argv[]) {
    return syscall2(SYSCALL_SPAWN, (uint32_t) path, (uint32_t) argv);
}
