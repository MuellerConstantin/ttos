#include <devio.h>
#include <syscall.h>

int32_t devio_list(uint32_t index, devinfo_t* info) {
    return syscall2(SYSCALL_LSDEV, index, (uint32_t) info);
}
