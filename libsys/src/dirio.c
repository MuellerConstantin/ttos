#include <dirio.h>
#include <syscall.h>

int32_t dirio_open(const char* path) {
    return syscall1(SYSCALL_OPENDIR, (uint32_t) path);
}

int32_t dirio_read(int32_t dd, dirent_t* entry) {
    return syscall2(SYSCALL_READDIR, (uint32_t) dd, (uint32_t) entry);
}

int32_t dirio_close(int32_t dd) {
    return syscall1(SYSCALL_CLOSEDIR, (uint32_t) dd);
}
