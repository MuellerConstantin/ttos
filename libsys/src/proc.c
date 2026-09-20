#include <proc.h>
#include <syscall.h>

char** environ = 0;

void _exit(int status) {
    syscall1(SYSCALL_EXIT, (uint32_t) status);
}

pid_t spawn(const char* path, char* const argv[], int flags) {
    return syscall4(SYSCALL_SPAWN, (uint32_t) path, (uint32_t) argv, (uint32_t) environ, (uint32_t) flags);
}

pid_t wait(pid_t pid, int* status, int options) {
    return syscall3(SYSCALL_WAIT, (uint32_t) pid, (uint32_t) status, (uint32_t) options);
}

int chdir(const char* path) {
    return syscall1(SYSCALL_CHDIR, (uint32_t) path);
}

int getcwd(char* buffer, size_t size) {
    return syscall2(SYSCALL_GETCWD, (uint32_t) buffer, (uint32_t) size);
}
