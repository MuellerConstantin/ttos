#include <mntio.h>
#include <syscall.h>

int32_t mntio_list(uint32_t index, mntinfo_t* info) {
    return syscall2(SYSCALL_LSMNT, index, (uint32_t) info);
}

int32_t mntio_mount(char drive, const char* id) {
    return syscall2(SYSCALL_MOUNT, (uint32_t) drive, (uint32_t) id);
}

int32_t mntio_unmount(char drive) {
    return syscall1(SYSCALL_UNMOUNT, (uint32_t) drive);
}
