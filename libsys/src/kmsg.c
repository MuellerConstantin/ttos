#include <kmsg.h>
#include <syscall.h>

int32_t kmsg_read(uint32_t index, kmsg_entry_t* entry) {
    return syscall2(SYSCALL_DMESG, index, (uint32_t) entry);
}
