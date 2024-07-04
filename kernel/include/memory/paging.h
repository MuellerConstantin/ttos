#ifndef _KERNEL_MEMORY_PAGING_H
#define _KERNEL_MEMORY_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <kernel.h>

#define PAGE_SIZE 4096

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

void paging_init();

#endif // _KERNEL_MEMORY_PAGING_H
