/**
 * @file paging.h
 * @brief Definitions for the kernel's paging system.
 * 
 * The paging system is responsible for managing the virtual memory of the kernel. It provides
 * functions to map and unmap memory regions, switch page directories, and copy page directories.
 */

#ifndef _KERNEL_MEMORY_PAGING_H
#define _KERNEL_MEMORY_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <memory/map.h>

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE 1024
#define PAGE_DIRECTORY_SIZE 1024

#define PAGE_DIRECTORY_INDEX(virtual_address) (((uint32_t)(virtual_address)) >> 22)
#define PAGE_TABLE_INDEX(virtual_address) ((((uint32_t)(virtual_address)) >> 12) & 0x3FF)
#define PAGE_OFFSET(virtual_address) ((uint32_t)(virtual_address) & 0xFFF)

struct page_table_entry {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t attribute_table : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t page_base : 20;
} __attribute__((packed));

typedef struct page_table_entry page_table_entry_t;

struct page_table {
    page_table_entry_t entries[1024];
} __attribute__((packed));

typedef struct page_table page_table_t;

struct page_directory_entry {
    uint32_t present : 1;
    uint32_t read_write : 1;
    uint32_t user_supervisor : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t page_size : 1;
    uint32_t global : 1;
    uint32_t available : 3;
    uint32_t page_table_base : 20;
} __attribute__((packed));

typedef struct page_directory_entry page_directory_entry_t;

struct page_directory {
    page_directory_entry_t entries[1024];

    /**
     * Array of pointers to page tables. Instead of the actual page directory
     * entries, we use this array to store the page tables virtual addresses.
     */
    page_table_t* tables[1024];
} __attribute__((packed));

typedef struct page_directory page_directory_t;

/**
 * Page directory used for prepaging during kernel initialization to
 * allow to load the kernel into the higher half of the address space.
 */
extern const page_directory_t *const prepaging_page_directory;

/**
 * Initialize the paging system.
 */
void paging_init();

/**
 * Get the current page directory.
 * 
 * @return The current page directory.
 */
page_directory_t* paging_get_current_page_directory();

/**
 * Map given virtual memory to some physical memory.
 * 
 * @param virtual_address The virtual address to map.
 * @param size The size of the memory to map.
 * @param physical_address Optional physical address to map to.
 * @param is_kernel Whether the memory is kernel memory.
 * @param is_writeable Whether the memory is writeable.
 */
void paging_map_memory(void *const virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable);

/**
 * Unmap given virtual memory.
 * 
 * @param virtual_address The virtual address to unmap.
 * @param size The size of the memory to unmap.
 */
void paging_unmap_memory(void *const virtual_address, size_t size);

/**
 * Switch to the given page directory.
 * 
 * @param page_directory The page directory to switch to.
 */
void paging_switch_page_directory(page_directory_t* page_directory);

/**
 * Copy the given page directory.
 * 
 * @param src_page_directory The page directory to copy.
 * @return The copied page directory.
 */
page_directory_t* paging_copy_page_directory(page_directory_t* src_page_directory);

#endif // _KERNEL_MEMORY_PAGING_H
