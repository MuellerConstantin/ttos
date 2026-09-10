#include <fsio.h>
#include <syscall.h>

int32_t fsio_read(int32_t fd, void* buffer, size_t size) {
    return syscall3(SYSCALL_READ, (uint32_t) fd, (uint32_t) buffer, size);
}

int32_t fsio_write(int32_t fd, const void* buffer, size_t size) {
    return syscall3(SYSCALL_WRITE, (uint32_t) fd, (uint32_t) buffer, size);
}

int32_t fsio_open(const char* path, int32_t flags, int32_t mode) {
    return syscall3(SYSCALL_OPEN, (uint32_t) path, (uint32_t) flags, (uint32_t) mode);
}

int32_t fsio_unlink(const char* path) {
    return syscall1(SYSCALL_UNLINK, (uint32_t) path);
}

int32_t fsio_mkdir(const char* path, int32_t mode) {
    return syscall2(SYSCALL_MKDIR, (uint32_t) path, (uint32_t) mode);
}

int32_t fsio_rmdir(const char* path) {
    return syscall1(SYSCALL_RMDIR, (uint32_t) path);
}

int32_t fsio_close(int32_t fd) {
    return syscall1(SYSCALL_CLOSE, (uint32_t) fd);
}
