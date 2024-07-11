#ifndef _KERNEL_SYS_KPANIC_H
#define _KERNEL_SYS_KPANIC_H

#include <stdint.h>
#include <stddef.h>
#include <sys/isr.h>
#include <drivers/video/vga/textmode.h>

#define KPANIC(code, message, info) kpanic(code, message, __FILE__, __LINE__, info)

#define KPANIC_CPU_EXCEPTION_TYPE(code)         (0x100000 | (code))
#define KPANIC_MULTIBOOT_EXCEPTION_TYPE(code)   (0x200000 | (code))
#define KPANIC_MEMORY_EXCEPTION_TYPE(code)      (0x300000 | (code))
#define KPANIC_FILESYSTEM_EXCEPTION_TYPE(code)  (0x400000 | (code))

// Multiboot exceptions

#define KPANIC_INVALID_MULTIBOOT_SIGNATURE_CODE         KPANIC_MULTIBOOT_EXCEPTION_TYPE(0)
#define KPANIC_INVALID_MULTIBOOT_SIGNATURE_MESSAGE      "Invalid Multiboot signature"

#define KPANIC_NO_MEMORY_MAP_CODE       KPANIC_MULTIBOOT_EXCEPTION_TYPE(1)
#define KPANIC_NO_MEMORY_MAP_MESSAGE    "No memory map provided by Multiboot"

#define KPANIC_NO_MODULES_PROVIDED_CODE         KPANIC_MULTIBOOT_EXCEPTION_TYPE(2)
#define KPANIC_NO_MODULES_PROVIDED_MESSAGE      "No modules provided by Multiboot"

// Memory exceptions

#define KPANIC_RAM_MINIMAL_SIZE_CODE           KPANIC_MEMORY_EXCEPTION_TYPE(0)
#define KPANIC_RAM_MINIMAL_SIZE_MESSAGE        "RAM size is less than the minimal required size"

#define KPANIC_PMM_OUT_OF_MEMORY_CODE           KPANIC_MEMORY_EXCEPTION_TYPE(1)
#define KPANIC_PMM_OUT_OF_MEMORY_MESSAGE        "Physical memory manager out of memory"

#define KPANIC_KHEAP_OUT_OF_MEMORY_CODE         KPANIC_MEMORY_EXCEPTION_TYPE(2)
#define KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE      "Kernel heap out of memory"

// Filesystem exceptions

#define KPANIC_INITRD_INIT_FAILED_CODE          KPANIC_FILESYSTEM_EXCEPTION_TYPE(0)
#define KPANIC_INITRD_INIT_FAILED_MESSAGE       "Failed to initialize initial ramdisk"

/**
 * Panic handler for the kernel that displays a message on
 * the screen and halts the system.
 * 
 * @param code The panic code.
 * @param message The message to display.
 * @param file The file where the panic occurred.
 * @param line The line where the panic occurred.
 * @param state The optional CPU state at the time of the panic.
 */
void kpanic(uint32_t code, const char *message, const char* file, uint32_t line, isr_cpu_state_t *state);

#endif // _KERNEL_SYS_KPANIC_H
