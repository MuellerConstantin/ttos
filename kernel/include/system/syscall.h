#ifndef _KERNEL_SYSTEM_SYSCALL_H
#define _KERNEL_SYSTEM_SYSCALL_H

#include <stdint.h>

#define SYSCALL_WRITE 0x01
#define SYSCALL_GET_OSINFO 0x04
#define SYSCALL_GET_MEMINFO 0x05

/**
 * Initializes the syscall handler.
 */
void syscall_init();

#endif // _KERNEL_SYSTEM_SYSCALL_H
