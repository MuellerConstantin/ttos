/**
 * @file vmm.h
 * @brief Virtual Memory Manager logic and structures.
 * 
 * This file contains the logic and structures for the Virtual Memory Manager. The VMM is responsible for
 * managing the virtual memory of the system.
 * 
 */

#ifndef _KERNEL_MEMORY_VMM_H
#define _KERNEL_MEMORY_VMM_H

#include <stdint.h>
#include <stddef.h>
#include <memory/pmm.h>
#include <memory/paging.h>

#define VMM_VM_SIZE 0xFFFFFFFF

/*
 * The kernel's virtual memory layout:
 * 
 * 0x00000000 - 0x00001000 : NULL pointer dereference
 * 0x00001000 - 0xBFFFF000 : User space
 * 0xC0000000 - 0xFFFFFFFF : Kernel space
 *      0xC0000000 - 0xC00FFFFF : Lower Memory (Real Mode Address Space)
 *          0xC00A0000 - 0xC00BFFFF : Video RAM
 *          0xC00C0000 - 0xC00C7FFF : Video BIOS
 *      0xC0100000 - 0xCFFFFFFF : Higher half Kernel (Code/Data/BSS)
 *      0xD0000000 - 0xFFFFFFFF : Kernel free use (Heap, DMA, etc.)
 */

#define VMM_USER_SPACE_BASE 0x00001000
#define VMM_USER_SPACE_SIZE 0xBFFFF000

#define VMM_KERNEL_SPACE_BASE 0xC0000000
#define VMM_KERNEL_SPACE_SIZE 0x3FFFFFFF

#define VMM_LOWER_MEMORY_BASE 0xC0000000
#define VMM_LOWER_MEMORY_SIZE 0x00100000

#define VMM_HIGHER_HALF_BASE 0xC0100000
#define VMM_HIGHER_HALF_SIZE 0x0FF00000

extern const uint32_t kernel_physical_start;
extern const uint32_t kernel_physical_end;
extern const uint32_t kernel_virtual_start;
extern const uint32_t kernel_virtual_end;

// Addresses of fixed memory regions

/** Virtual base address of kernel heap. */
#define VMM_KERNEL_HEAP_BASE 0xE0000000

/** Size of kernel heap. */
#define VMM_KERNEL_HEAP_SIZE 0x4000000

/** Virtual base address of AHCI/SATA DMA buffer. */
#define VMM_SATA_DMA_BUFFER_BASE 0xF0000000

/** Size of AHCI/SATA DMA buffer. */
#define VMM_SATA_DMA_BUFFER_SIZE 0x100000

/**
 * Initialize the Virtual Memory Manager.
 */
void vmm_init();

/**
 * Map a memory region to a virtual address.
 * 
 * @param virtual_address The virtual address to map the memory to.
 * @param size The size of the memory region.
 * @param physical_address The physical address of the memory region.
 * @param is_kernel Whether the memory is kernel memory.
 * @param is_writeable Whether the memory is writeable.
 * @return 0 if the memory was mapped successfully, -1 otherwise.
 */
int32_t vmm_map_memory(void *const virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable);

/**
 * Unmap a memory region from a virtual address.
 * 
 * @param virtual_address The virtual address to unmap the memory from.
 * @param size The size of the memory region.
 * @return 0 if the memory was unmapped successfully, -1 otherwise.
 */
int32_t vmm_unmap_memory(void *const virtual_address, size_t size);

#endif // _KERNEL_MEMORY_VMM_H
