#include <memory/paging.h>
#include <sys/kpanic.h>
#include <memory/kheap.h>
#include <memory/pmm.h>
#include <memory/vmm.h>

static bool paging_enabled = false;

void paging_enable() {
    uint32_t cr4;
    uint32_t cr0;

    __asm__ volatile("mov %%cr4, %0" : "=r" (cr4));
    __asm__ volatile("mov %%cr0, %0" : "=r" (cr0));

    cr4 &= 0xFFFFFFeF; // Disable 4MB pages
    cr0 |= 0x80000000; // Enable paging

    __asm__ volatile("mov %0, %%cr4" : : "r" (cr4));
    __asm__ volatile("mov %0, %%cr0" : : "r" (cr0));

    paging_enabled = true;
}

void paging_switch_page_directory(page_directory_t* current_page_directory, page_directory_t* new_page_directory) {
    uintptr_t physical_address = (uintptr_t) paging_virtual_to_physical_address(current_page_directory, new_page_directory);
    __asm__ volatile("mov %0, %%cr3" : : "r" (physical_address));
}

int32_t paging_allocate_page(page_directory_t *const page_directory, void *const virtual_address, void* frame_address, bool is_kernel, bool is_writeable) {
    page_table_t *table = NULL;
    
    uint32_t page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    uint32_t page_table_index = PAGE_TABLE_INDEX(virtual_address);

    // Allocate a new page table if it does not exist
    if(page_directory->tables[page_directory_index] == NULL) {
        table = (page_table_t*) kmalloc_a(sizeof(page_table_t));

        if(!table) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        memset(table, 0, sizeof(page_table_t));

        uint32_t table_physical_address = (uint32_t) paging_virtual_to_physical_address(page_directory, table);

        page_directory->entries[page_directory_index].present = 1;
        page_directory->entries[page_directory_index].read_write = 1;
        page_directory->entries[page_directory_index].user_supervisor = 1;
        page_directory->entries[page_directory_index].page_table_base = table_physical_address >> 12;
        page_directory->entries[page_directory_index].page_size = 0;

        page_directory->tables[page_directory_index] = table;
    } else {
        table = page_directory->tables[page_directory_index];
    }

    // Allocate a new page if it does not exist
    if(!table->entries[page_table_index].present) {
        uint32_t frame_index = pmm_address_to_index(frame_address);

        table->entries[page_table_index].present = 1;
        table->entries[page_table_index].read_write = is_writeable ? 1 : 0;
        table->entries[page_table_index].user_supervisor = is_kernel ? 0 : 1;
        table->entries[page_table_index].page_base = frame_index;

        return 0;
    }

    return -1;
}

void* paging_free_page(page_directory_t *const page_directory, void *const virtual_address) {
    uint32_t page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    uint32_t page_table_index = PAGE_TABLE_INDEX(virtual_address);

    if(!page_directory->tables[page_directory_index]) {
        return;
    }

    page_table_t *table = page_directory->tables[page_directory_index];

    if(!table->entries[page_table_index].present) {
        return;
    }

    void* frame_address = pmm_index_to_address(table->entries[page_table_index].page_base);

    table->entries[page_table_index].present = 0;
    table->entries[page_table_index].page_base = 0;

    return frame_address;
}

void* paging_virtual_to_physical_address(const page_directory_t *const page_directory, void *const virtual_address) {
    uint32_t page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    uint32_t page_table_index = PAGE_TABLE_INDEX(virtual_address);
    uint32_t page_offset = PAGE_OFFSET(virtual_address);

    if(!paging_enabled) {
        return (void*) (virtual_address - VMM_KERNEL_SPACE_BASE);
    }

    if (!page_directory->tables[page_directory_index]) {
        return NULL;
    }

    page_table_t *table = page_directory->tables[page_directory_index];

    if (!table->entries[page_table_index].present) {
        return NULL;
    }

    return (void*) ((table->entries[page_table_index].page_base << 12) + page_offset);
}
