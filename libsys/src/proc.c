#include <proc.h>
#include <syscall.h>

char** environ = 0;

void _exit(int status) {
    syscall1(SYSCALL_EXIT, (uint32_t) status);
}

int spawn(const char* path, char* const argv[]) {
    return syscall3(SYSCALL_SPAWN, (uint32_t) path, (uint32_t) argv, (uint32_t) environ);
}
