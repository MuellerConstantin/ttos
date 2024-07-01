#ifndef _KERNEL_SYS_PANIC_H
#define _KERNEL_SYS_PANIC_H

#include <stdint.h>
#include <stddef.h>
#include <sys/isr.h>
#include <drivers/video/vga/textmode.h>

/**
 * Panic handler for the kernel that displays a message on
 * the screen and halts the system.
 * 
 * @param msg The message to display
 * @param state The optional CPU state at the time of the panic
 */
void kpanic(const char* msg, isr_cpu_state_t *state);

#endif // _KERNEL_SYS_PANIC_H
