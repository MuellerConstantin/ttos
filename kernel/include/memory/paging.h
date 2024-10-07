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
 * Enable paging.
 */
void paging_enable();

/**
 * Allocate a page in the given page directory.
 * 
 * @param page_directory The page directory to allocate the page in.
 * @param page_address The virtual address to allocate the page at.
 * @param frame_address The physical address of the frame to allocate.
 * @param is_kernel Whether the memory is kernel memory.
 * @param is_writeable Whether the memory is writeable.
 * @return 0 if the page was allocated successfully, -1 otherwise.
 */
int32_t paging_allocate_page(page_directory_t *const page_directory, void *const page_address, void* frame_address, bool is_kernel, bool is_writeable);

/**
 * Free a page in the given page directory.
 * 
 * @param page_directory The page directory to free the page in.
 * @param page_address The virtual address of the page to free.
 * @return The physical address of the freed page.
 */
void* paging_free_page(page_directory_t *const page_directory, void *const page_address);

/**
 * Switch to the given page directory.
 * 
 * @param current_page_directory The current page directory.
 * @param new_page_directory The new page directory to switch to.
 */
void paging_switch_page_directory(page_directory_t* current_page_directory, page_directory_t* new_page_directory);

/**
 * translate a virtual address to a physical address.
 * 
 * @param page_directory The page directory to use for translation.
 * @param virtual_address The virtual address to translate.
 * @return The physical address of the virtual address.
 */
void* paging_virtual_to_physical_address(const page_directory_t *const page_directory, void *const virtual_address);

#endif // _KERNEL_MEMORY_PAGING_H
