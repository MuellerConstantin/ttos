#include <devio.h>
#include <syscall.h>

int32_t devio_list(uint32_t index, devinfo_t* info) {
    return syscall2(SYSCALL_LSDEV, index, (uint32_t) info);
}

int32_t devio_read(const char* id, size_t offset, void* buffer, size_t size) {
    return syscall4(SYSCALL_DEVREAD, (uint32_t) id, offset, size, (uint32_t) buffer);
}

int32_t devio_write(const char* id, size_t offset, const void* buffer, size_t size) {
    return syscall4(SYSCALL_DEVWRITE, (uint32_t) id, offset, size, (uint32_t) buffer);
}

int32_t devio_rescan(const char* id) {
    return syscall1(SYSCALL_RESCAN, (uint32_t) id);
}
