#include <volio.h>
#include <syscall.h>

int32_t volio_list(uint32_t index, volinfo_t* info) {
    return syscall2(SYSCALL_LSVOL, index, (uint32_t) info);
}

int32_t volio_read(const char* id, size_t offset, void* buffer, size_t size) {
    return syscall4(SYSCALL_VOLREAD, (uint32_t) id, offset, size, (uint32_t) buffer);
}

int32_t volio_write(const char* id, size_t offset, const void* buffer, size_t size) {
    return syscall4(SYSCALL_VOLWRITE, (uint32_t) id, offset, size, (uint32_t) buffer);
}
