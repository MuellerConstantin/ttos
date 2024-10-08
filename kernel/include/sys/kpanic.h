/**
 * @file kpanic.h
 * @brief Kernel panic handler.
 * 
 * This file contains the definitions for the kernel panic handler. The panic handler is used to
 * display a message on the screen and halt the system when a critical error occurs.
 */

#ifndef _KERNEL_SYS_KPANIC_H
#define _KERNEL_SYS_KPANIC_H

#include <stdint.h>
#include <stddef.h>
#include <sys/isr.h>
#include <drivers/video/vga/vga.h>
#include <drivers/video/vga/tm.h>

#define KPANIC(code, message, info) kpanic(code, message, __FILE__, __LINE__, info)

#define KPANIC_CPU_EXCEPTION_TYPE(code)             (0x000000 | (code))
#define KPANIC_MULTIBOOT_EXCEPTION_TYPE(code)       (0x100000 | (code))
#define KPANIC_INFRASTRUCTURE_EXCEPTION_TYPE(code)  (0x200000 | (code))
#define KPANIC_MEMORY_EXCEPTION_TYPE(code)          (0x300000 | (code))

/*
 * Multiboot exceptions
 *
 * Multiboot exceptions are used if the bootloader does not provide the necessary information
 * for the kernel to boot properly.
 */

#define KPANIC_INVALID_MULTIBOOT_SIGNATURE_CODE         KPANIC_MULTIBOOT_EXCEPTION_TYPE(0)
#define KPANIC_INVALID_MULTIBOOT_SIGNATURE_MESSAGE      "Invalid Multiboot signature"

#define KPANIC_NO_MEMORY_MAP_CODE                       KPANIC_MULTIBOOT_EXCEPTION_TYPE(1)
#define KPANIC_NO_MEMORY_MAP_MESSAGE                    "No memory map provided by Multiboot"

#define KPANIC_NO_MODULES_PROVIDED_CODE                 KPANIC_MULTIBOOT_EXCEPTION_TYPE(2)
#define KPANIC_NO_MODULES_PROVIDED_MESSAGE              "No module information provided by Multiboot"

/*
 * Infrastructure exceptions
 *
 * Infrastructure exceptions are used if a device, required by the kernel to function properly, or
 * a required volume is not found. This can be an input or output device.
 */

#define KPANIC_NO_INITRD_DEVICE_FOUND_CODE      KPANIC_INFRASTRUCTURE_EXCEPTION_TYPE(0)
#define KPANIC_NO_INITRD_DEVICE_FOUND_MESSAGE   "No initial ramdisk device/volume found"

#define KPANIC_INITRD_MOUNT_FAILED_CODE         KPANIC_INFRASTRUCTURE_EXCEPTION_TYPE(1)
#define KPANIC_INITRD_MOUNT_FAILED_MESSAGE      "Failed to mount initial ramdisk"

#define KPANIC_NO_INPUT_DEVICE_FOUND_CODE       KPANIC_INFRASTRUCTURE_EXCEPTION_TYPE(2)
#define KPANIC_NO_INPUT_DEVICE_FOUND_MESSAGE    "No input device found"

#define KPANIC_NO_OUTPUT_DEVICE_FOUND_CODE      KPANIC_INFRASTRUCTURE_EXCEPTION_TYPE(3)
#define KPANIC_NO_OUTPUT_DEVICE_FOUND_MESSAGE   "No output device found"

/*
 * Memory exceptions
 *
 * Memory exceptions are used if the kernel runs out of memory or if the memory is not sufficient
 * for the kernel to boot properly.
 */

#define KPANIC_RAM_MINIMAL_SIZE_CODE            KPANIC_MEMORY_EXCEPTION_TYPE(0)
#define KPANIC_RAM_MINIMAL_SIZE_MESSAGE         "RAM size is less than the minimal required size"

#define KPANIC_PMM_OUT_OF_MEMORY_CODE           KPANIC_MEMORY_EXCEPTION_TYPE(1)
#define KPANIC_PMM_OUT_OF_MEMORY_MESSAGE        "Physical memory manager out of memory"

#define KPANIC_KHEAP_OUT_OF_MEMORY_CODE         KPANIC_MEMORY_EXCEPTION_TYPE(2)
#define KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE      "Kernel heap out of memory"

#define KPANIC_VMM_OUT_OF_USER_SPACE_CODE       KPANIC_MEMORY_EXCEPTION_TYPE(3)
#define KPANIC_VMM_OUT_OF_USER_SPACE_MESSAGE    "Out of user space memory"

#define KPANIC_VMM_OUT_OF_KERNEL_SPACE_CODE     KPANIC_MEMORY_EXCEPTION_TYPE(4)
#define KPANIC_VMM_OUT_OF_KERNEL_SPACE_MESSAGE  "Out of kernel space memory"

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
