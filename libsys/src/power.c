#include <power.h>
#include <syscall.h>

int32_t power_off(void) {
    return syscall0(SYSCALL_POWEROFF);
}
