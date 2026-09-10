#include <volio.h>
#include <syscall.h>

int32_t volio_list(uint32_t index, volinfo_t* info) {
    return syscall2(SYSCALL_LSVOL, index, (uint32_t) info);
}
