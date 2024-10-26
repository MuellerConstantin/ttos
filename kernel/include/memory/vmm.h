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
#include <stdbool.h>
#include <arch/i386/paging.h>

#define VMM_VAS_SIZE 0xFFFFFFFF

#define VMM_IS_ALIGNED(address) (((uintptr_t) address % PAGE_SIZE) == 0)
#define VMM_ALIGN(address) ((void*) ((uintptr_t) address & ~(PAGE_SIZE - 1)))
#define VMM_OFFSET(address) ((uintptr_t) address % PAGE_SIZE)

/*
 * The kernel's virtual memory layout:
 * 
 * 0x00000000 - 0x00001000 : NULL pointer dereference
 * 0x00001000 - 0xBFFFF000 : User space
 * 0xC0000000 - 0xFFFFFFFF : Kernel space
 *      0xC0000000 - 0xC00FFFFF : Real Mode Memory
 *          0xC00A0000 - 0xC00BFFFF : Video RAM
 *          0xC00C0000 - 0xC00C7FFF : Video BIOS
 *      0xC0100000 - 0xCFFFFFFF : Higher half Kernel (Code/Data/BSS)
 *      0xD0000000 - 0xFFFFFFFF : Kernel free use (Heap, DMA, etc.)
 */

#define VMM_USER_SPACE_BASE 0x00001000
#define VMM_USER_SPACE_SIZE 0xBFFFF000

#define VMM_KERNEL_SPACE_BASE 0xC0000000
#define VMM_KERNEL_SPACE_SIZE 0x3FFFFFFF

#define VMM_REAL_MODE_MEMORY_BASE   0xC0000000
#define VMM_REAL_MODE_MEMORY_SIZE   0x00100000

#define VMM_HIGHER_HALF_BASE 0xC0100000
#define VMM_HIGHER_HALF_SIZE 0x0FF00000

extern char kernel_physical_start[];
extern char kernel_physical_end[];
extern char kernel_virtual_start[];
extern char kernel_virtual_end[];

/**
 * Initialize the Virtual Memory Manager.
 */
void vmm_init();

/**
 * Map a memory region to a virtual address.
 * 
 * It is important to note that the VMM can only map whole pages, so the given
 * virtual/physical address should be page aligned and the size should be a multiple
 * of the page size. If that is not the case, the VMM will still reserve whole pages,
 * even if the given virtual address starts in the middle of a page or the size is not
 * a multiple of the page size.
 * 
 * This results in a always page aligned virtual address that is returned if no custom
 * virtual address is specified, even if the given physical address is not page aligned.
 * This means that the returned virtual address is the base address of the mapped memory
 * region and not necessarily the virtual address related to the physical address.
 * 
 * @param virtual_address Optional virtual address to map the memory to. If NULL, the VMM will find a free memory region.
 *        If the address is given, it will be made page aligned.
 * @param size The size of the memory region.
 * @param physical_address Optional physical address to map the memory from. If NULL, the VMM will allocate a new frame(s).
 * @param is_kernel Whether the memory is kernel memory and should be placed in virtual kernel space.
 * @param is_writeable Whether the memory is writeable.
 * @return The page aligned virtual address of the mapped memory region, or NULL if the memory region could not be mapped.
 */
void* vmm_map_memory(void* virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable);

/**
 * Unmap a memory region from a virtual address.
 * 
 * @param virtual_address The virtual address to unmap the memory from.
 * @param size The size of the memory region.
 */
void vmm_unmap_memory(void* virtual_address, size_t size);

/**
 * Check if a virtual address is mapped. More precisely, it checks if the page related
 * to the virtual address is mapped.
 */
bool vmm_is_mapped(void* virtual_address);

/**
 * Get the physical address of a mapped virtual address.
 * 
 * @param virtual_address The virtual address to get the physical address for.
 * @return The physical address of the virtual address, or NULL if the virtual address is not mapped.
 */
void* vmm_get_mapped_address(void* virtual_address);

#endif // _KERNEL_MEMORY_VMM_H
