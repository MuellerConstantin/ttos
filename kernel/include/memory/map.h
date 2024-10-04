/**
 * @file map.h
 * @brief Definitions for the kernel's memory map.
 * 
 * The memory map defines the layout of the kernel's virtual memory space and the physical memory
 * regions used by the kernel.
 */

#ifndef _KERNEL_MEMORY_MAP_H
#define _KERNEL_MEMORY_MAP_H

#include <stdint.h>

#define KERNEL_MINIMAL_PHYSICAL_RAM_SIZE 0x40000000

/*
 * The kernel's virtual memory layout:
 *
 * 0x00000000 - 0x00001000 : NULL pointer dereference
 * 0x00001000 - 0xC0000000 : User space
 * 0xC0000000 - 0xFFFFFFFF : Kernel space
 *      0xC0000000 - 0xD0000000 : Kernel's higher half
 *          0xC0000000 - 0xC00FFFFF : Real Mode Address Space
 *              0xC00A0000 - 0xC00BFFFF : Video RAM
 *              0xC00C0000 - 0xC00C7FFF : Video BIOS
 *          0xC0100000 - 0x???????? : Kernel (Code/Data/BSS)
 *      0xE0000000 - 0xE4000000 : Kernel Heap
 */

/** Virtual base address of the kernel space. */
#define KERNEL_SPACE_VIRTUAL_BASE 0xC0000000

/** Size of the kernel space. */
#define KERNEL_SPACE_VIRTUAL_SIZE 0x40000000

/** Virtual base address of the user space. */
#define USER_SPACE_VIRTUAL_BASE 0x00001000

/** Size of the user space. */
#define USER_SPACE_VIRTUAL_SIZE 0xC0000000

/** The virtual base address of the kernel's higher half. */
#define KERNEL_HIGHER_HALF_VIRTUAL_BASE 0xC0000000

/** Size of kernel's higher half space. */
#define KERNEL_HIGHER_HALF_VIRTUAL_SIZE 0x10000000

/** Virtual base address of kernel heap. */
#define KERNEL_HEAP_VIRTUAL_BASE 0xE0000000

/** Size of kernel heap. */
#define KERNEL_HEAP_VIRTUAL_SIZE 0x4000000

extern const uint32_t kernel_physical_start;
extern const uint32_t kernel_physical_end;
extern const uint32_t kernel_virtual_start;
extern const uint32_t kernel_virtual_end;

#endif // _KERNEL_MEMORY_MAP_H
