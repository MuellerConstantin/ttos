#include <memmap.h>
#include <syscall.h>

int32_t memmap_read(uint32_t index, memregion_t* region) {
    return syscall2(SYSCALL_MEMMAP, index, (uint32_t) region);
}
