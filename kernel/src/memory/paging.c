#include <memory/paging.h>
#include <sys/kpanic.h>
#include <memory/kheap.h>
#include <memory/pmm.h>
#include <drivers/serial/uart/16550.h>

static page_directory_t *kernel_page_directory = NULL;
static page_directory_t *current_page_directory = NULL;

static bool paging_enabled = false;

static void* paging_virtual_to_physical_address(const page_directory_t *const page_directory, void *const virtual_address);
static void paging_allocate_page(page_directory_t *const page_directory, void *const virtual_address, void* frame_address, bool is_kernel, bool is_writeable);
static void paging_free_page(page_directory_t *const page_directory, void *const virtual_address);
static void paging_copy_page_table(page_directory_t* src_page_directory, page_directory_t* dst_page_directory, uint32_t page_directory_index);
static void paging_enable();

void paging_init() {
    // Technically, paging is already enabled by the boot section
    current_page_directory = prepaging_page_directory;

    kernel_page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));

    if(!kernel_page_directory) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    // Mapping the kernel's virtual address space
    for(uint32_t page_address = KERNEL_SPACE_BASE;
        page_address < KERNEL_SPACE_BASE + KERNEL_SPACE_SIZE;
        page_address += PAGE_SIZE) {
        paging_allocate_page(kernel_page_directory, (void*) page_address, (void*) (page_address - KERNEL_SPACE_BASE), true, true);
    }

    paging_switch_page_directory(kernel_page_directory);

    paging_enable();
}

page_directory_t* paging_get_current_page_directory() {
    return current_page_directory;
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

void paging_switch_page_directory(page_directory_t* page_directory) {
    current_page_directory = page_directory;
    uintptr_t physical_address = (uintptr_t) paging_virtual_to_physical_address(current_page_directory, page_directory);
    __asm__ volatile("mov %0, %%cr3" : : "r" (physical_address));
}

void paging_map_memory(void *const virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable) {
    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
        page_address += PAGE_SIZE) {
        
        if(physical_address) {
            paging_allocate_page(current_page_directory, (void*) page_address, physical_address, is_kernel, is_writeable);
            physical_address += PAGE_SIZE;
        } else {
            paging_allocate_page(current_page_directory, (void*) page_address, NULL, is_kernel, is_writeable);
        }
    }
}

void paging_unmap_memory(void *const virtual_address, size_t size) {
    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
        page_address += PAGE_SIZE) {
        
        paging_free_page(current_page_directory, (void*) page_address);
    }
}

static void paging_allocate_page(page_directory_t *const page_directory, void *const virtual_address, void* frame_address, bool is_kernel, bool is_writeable) {
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
        // If no frame address is provided, allocate a new frame
        if(!frame_address) {
            frame_address = pmm_alloc_frame();

            if(!frame_address) {
                KPANIC(KPANIC_PMM_OUT_OF_MEMORY_CODE, KPANIC_PMM_OUT_OF_MEMORY_MESSAGE, NULL);
            }
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

page_directory_t* paging_copy_page_directory(page_directory_t* src_page_directory) {
    page_directory_t* dst_page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));

    if(!dst_page_directory) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    for(size_t index = 0; index < PAGE_DIRECTORY_SIZE; index++) {
        if(!src_page_directory->tables[index]) {
            continue;
        }

        // Link Kernel pages
        if(kernel_page_directory->tables[index] == src_page_directory->tables[index]) {
            dst_page_directory->entries[index] = src_page_directory->entries[index];
            dst_page_directory->tables[index] = src_page_directory->tables[index];
        // Copy non-kernel pages, to allow forked processes to have their own page tables
        } else {
            paging_copy_page_table(src_page_directory, dst_page_directory, index);
        }
    }

    return dst_page_directory;
}

static void paging_copy_page_table(page_directory_t* src_page_directory,
    page_directory_t* dst_page_directory,
    uint32_t page_directory_index) {
    // Copies the entries from the source page table to the new page table
    for(size_t index = 0; index < PAGE_TABLE_SIZE; index++) {
        if(!src_page_directory->tables[page_directory_index]->entries[index].present) {
            continue;
        }

        uintptr_t src_page_virtual_address = (page_directory_index << 22) | (index << 12) | 0;
        uintptr_t dst_page_virtual_address = src_page_virtual_address;
        uintptr_t tmp_page_virtual_address = 0;

        /**
         * First of all the virtual address of source page is also mapped to a phyiscal frame
         * in the destination page directory. After that we need a temporary address mapping
         * in the source page directory to this physical frame. This is necessary to copy the
         * content of the page to the new page.
         */

        paging_allocate_page(dst_page_directory, (void*) dst_page_virtual_address, NULL, false, true);

        uintptr_t dst_page_physical_address = (uintptr_t) paging_virtual_to_physical_address(dst_page_directory, (void*) dst_page_virtual_address);

        paging_allocate_page(src_page_directory, (void*) tmp_page_virtual_address, (void*) dst_page_physical_address, false, true);

        // Copy the content of the page
        memcpy((void*) tmp_page_virtual_address, (void*) src_page_virtual_address, PAGE_SIZE);

        dst_page_directory->tables[page_directory_index]->entries[index].present = src_page_directory->tables[page_directory_index]->entries[index].present;
        dst_page_directory->tables[page_directory_index]->entries[index].read_write = src_page_directory->tables[page_directory_index]->entries[index].read_write;
        dst_page_directory->tables[page_directory_index]->entries[index].user_supervisor = src_page_directory->tables[page_directory_index]->entries[index].user_supervisor;
        dst_page_directory->tables[page_directory_index]->entries[index].accessed = src_page_directory->tables[page_directory_index]->entries[index].accessed;
        dst_page_directory->tables[page_directory_index]->entries[index].dirty = src_page_directory->tables[page_directory_index]->entries[index].dirty;

        // Unmap the temporary address
        paging_free_page(src_page_directory, (void*) tmp_page_virtual_address);
    }
}
