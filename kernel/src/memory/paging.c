#include <memory/paging.h>
#include <memory/kheap.h>
#include <memory/pmm.h>

static page_directory_t *kernel_page_directory = NULL;

static bool paging_enabled = false;

static void* paging_virtual_to_physical_address(const page_directory_t *const page_directory, void *const virtual_address);
static void paging_allocate_page(page_directory_t *const page_directory, void *const virtual_address, void* frame_address, bool is_kernel, bool is_writeable);
static void paging_free_page(page_directory_t *const page_directory, void *const virtual_address);
static void paging_switch_page_directory(page_directory_t* page_directory);
static void paging_enable();

void paging_init() {
    kernel_page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));
    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    // Mapping the kernel's virtual address space
    for(uint32_t page_address = KERNEL_SPACE_BASE;
        page_address < KERNEL_SPACE_BASE + KERNEL_SPACE_SIZE;
        page_address += PAGE_SIZE) {
        paging_allocate_page(kernel_page_directory, (void*) page_address, (void*) (page_address - KERNEL_SPACE_BASE), true, true);
    }

    uint32_t page_directory_physical_address = (uint32_t) paging_virtual_to_physical_address(prepaging_page_directory, kernel_page_directory);

    paging_switch_page_directory((page_directory_t*) page_directory_physical_address);

    paging_enable();
}

static void paging_enable() {
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

static void paging_switch_page_directory(page_directory_t* page_directory) {
    uint32_t physical_address = (uint32_t) page_directory;
    __asm__ volatile("mov %0, %%cr3" : : "r" (physical_address));
}

void paging_map_memory(void *const virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable) {
    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
        page_address += PAGE_SIZE) {
        
        if(physical_address) {
            paging_allocate_page(kernel_page_directory, (void*) page_address, physical_address, is_kernel, is_writeable);
            physical_address += PAGE_SIZE;
        } else {
            paging_allocate_page(kernel_page_directory, (void*) page_address, NULL, is_kernel, is_writeable);
        }
    }
}

void paging_unmap_memory(void *const virtual_address, size_t size) {
    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
        page_address += PAGE_SIZE) {
        
        paging_free_page(kernel_page_directory, (void*) page_address);
    }
}

static void paging_allocate_page(page_directory_t *const page_directory, void *const virtual_address, void* frame_address, bool is_kernel, bool is_writeable) {
    page_table_t *table = NULL;
    
    uint32_t page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    uint32_t page_table_index = PAGE_TABLE_INDEX(virtual_address);

    // Allocate a new page table if it does not exist
    if(page_directory->tables[page_directory_index] == NULL) {
        table = (page_table_t*) kmalloc_a(sizeof(page_table_t));
        memset(table, 0, sizeof(page_table_t));

        uint32_t table_physical_address = (uint32_t) paging_virtual_to_physical_address(page_directory, table);

        page_directory->entries[page_directory_index].present = 1;
        page_directory->entries[page_directory_index].read_write = 1;
        page_directory->entries[page_directory_index].user_supervisor = 0;
        page_directory->entries[page_directory_index].page_table_base = table_physical_address >> 12;
        page_directory->entries[page_directory_index].page_size = 0;

        page_directory->tables[page_directory_index] = table;
    } else {
        table = page_directory->tables[page_directory_index];
    }

    // Allocate a new page if it does not exist
    if(!table->entries[page_table_index].present) {
        // If no frame address is provided, allocate a new frame
        if(!frame_address) {
            frame_address = pmm_alloc_frame();
        } else {
            pmm_mark_region_reserved(frame_address, PAGE_SIZE);
        }

        uint32_t frame_index = pmm_address_to_index(frame_address);

        table->entries[page_table_index].present = 1;
        table->entries[page_table_index].read_write = is_writeable ? 1 : 0;
        table->entries[page_table_index].user_supervisor = is_kernel ? 0 : 1;
        table->entries[page_table_index].page_base = frame_index;
    }
}

static void paging_free_page(page_directory_t *const page_directory, void *const virtual_address) {
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
    pmm_free_frame(frame_address);

    table->entries[page_table_index].present = 0;
    table->entries[page_table_index].page_base = 0;
}

static void* paging_virtual_to_physical_address(const page_directory_t *const page_directory, void *const virtual_address) {
    uint32_t page_directory_index = PAGE_DIRECTORY_INDEX(virtual_address);
    uint32_t page_table_index = PAGE_TABLE_INDEX(virtual_address);
    uint32_t page_offset = PAGE_OFFSET(virtual_address);

    if(!paging_enabled) {
        return (void*) (virtual_address - KERNEL_SPACE_BASE);
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
