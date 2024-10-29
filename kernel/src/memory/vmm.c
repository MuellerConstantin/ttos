#include <memory/vmm.h>
#include <memory/pmm.h>
#include <arch/i386/paging.h>
#include <system/kpanic.h>
#include <memory/kheap.h>
#include <system/kmessage.h>

static page_directory_t *kernel_page_directory = NULL;
static page_directory_t *current_page_directory = NULL;

static void* vmm_map_page(void *const virtual_address, void* physical_address, bool is_kernel, bool is_writeable);
static void vmm_unmap_page(void *const virtual_address);
static void* vmm_find_free_memory(size_t size, bool is_kernel);

void vmm_init() {
    current_page_directory = prepaging_page_directory;

    kernel_page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));

    if(!kernel_page_directory) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    // Mapping real mode memory (Physical: 0x00000000 - 0x000FFFFF)

    for(uint32_t virtual_address = VMM_REAL_MODE_MEMORY_BASE;
        virtual_address < VMM_REAL_MODE_MEMORY_BASE + VMM_REAL_MODE_MEMORY_SIZE;
        virtual_address += PAGE_SIZE) {
        uint32_t physical_address = virtual_address - VMM_KERNEL_SPACE_BASE;

        paging_map_page(kernel_page_directory, (void*) virtual_address, (void*) physical_address, true, true);
        pmm_mark_frame_reserved((void*) physical_address);
    }

    // Mapping kernel memory (Physical: 0x00100000 - 0x????????)

    for(uint32_t virtual_address = (uint32_t) kernel_virtual_start, physical_address = (uint32_t) kernel_physical_start;
        virtual_address < (uint32_t) kernel_virtual_end;
        virtual_address += PAGE_SIZE, physical_address += PAGE_SIZE) {
        
        paging_map_page(kernel_page_directory, (void*) virtual_address, (void*) physical_address, true, true);
        pmm_mark_frame_reserved((void*) physical_address);
    }

    paging_switch_page_directory(current_page_directory, kernel_page_directory);

    current_page_directory = kernel_page_directory;

    /*
     * Technically, paging should be already enabled by the kernel's startup code. However, we
     * still need to enable it here to ensure that paging is actually activated and more important
     * to ensure that the kernel's internal state is correct. This is required for correct virtual
     * to physical software address translation.
     */

    paging_enable();

    kmessage(KMESSAGE_LEVEL_INFO, "memory: VMM initialized");
}

void* vmm_map_memory(void* virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable) {
    uint32_t frame_address = (uint32_t) physical_address;

    if(!virtual_address) {
        virtual_address = vmm_find_free_memory(size, is_kernel);

        if(!virtual_address) {
            KPANIC(is_kernel ? KPANIC_VMM_OUT_OF_KERNEL_SPACE_MEMORY_CODE : KPANIC_VMM_OUT_OF_USER_SPACE_MEMORY_CODE,
                   is_kernel ? KPANIC_VMM_OUT_OF_KERNEL_SPACE_MEMORY_MESSAGE : KPANIC_VMM_OUT_OF_USER_SPACE_MEMORY_MESSAGE,
                   NULL);
        }
    } else {
        // Ensure given virtual address is within kernel space if it is kernel memory
        if(is_kernel && virtual_address < VMM_KERNEL_SPACE_BASE) {
            KPANIC(KPANCI_VMM_MEMORY_SPACE_BOUNDS_VIOLATION_CODE, KPANCI_VMM_MEMORY_SPACE_BOUNDS_VIOLATION_MESSAGE, NULL);
        }

        // Ensure given virtual address is within user space if it is user memory
        if(!is_kernel && (uint32_t) virtual_address + size >= VMM_KERNEL_SPACE_BASE) {
            KPANIC(KPANCI_VMM_MEMORY_SPACE_BOUNDS_VIOLATION_CODE, KPANCI_VMM_MEMORY_SPACE_BOUNDS_VIOLATION_MESSAGE, NULL);
        }
    }

    // Page align the virtual address
    virtual_address = VMM_ALIGN_DOWN(virtual_address);

    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < ((uint32_t) virtual_address) + size;
        page_address += PAGE_SIZE) {
        
        if(frame_address) {
            vmm_map_page((void*) page_address, (void*) frame_address, is_kernel, is_writeable);
            frame_address += PAGE_SIZE;
        } else {
            vmm_map_page((void*) page_address, NULL, is_kernel, is_writeable);
        }
    }

    return virtual_address;
}

static void* vmm_map_page(void* virtual_address, void* physical_address, bool is_kernel, bool is_writeable) {
    // If no frame address is provided, allocate a new frame
    if(!physical_address) {
        physical_address = pmm_alloc_frame();

        if(!physical_address) {
            KPANIC(KPANIC_PMM_OUT_OF_MEMORY_CODE, KPANIC_PMM_OUT_OF_MEMORY_MESSAGE, NULL);
        }
    } else {
        pmm_mark_frame_reserved(physical_address);
    }

    if(!virtual_address) {
        virtual_address = vmm_find_free_memory(PAGE_SIZE, is_kernel);

        if(!virtual_address) {
            KPANIC(is_kernel ? KPANIC_VMM_OUT_OF_KERNEL_SPACE_MEMORY_CODE : KPANIC_VMM_OUT_OF_USER_SPACE_MEMORY_CODE,
                   is_kernel ? KPANIC_VMM_OUT_OF_KERNEL_SPACE_MEMORY_MESSAGE : KPANIC_VMM_OUT_OF_USER_SPACE_MEMORY_MESSAGE,
                   NULL);
        }
    }

    paging_map_page(current_page_directory, virtual_address, physical_address, is_kernel, is_writeable);

    return virtual_address;
}

void vmm_unmap_memory(void *const virtual_address, size_t size) {
    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
        page_address += PAGE_SIZE) {
        
        vmm_unmap_page((void*) page_address);
    }
}

static void vmm_unmap_page(void *const virtual_address) {
    void* frame_address = paging_unmap_page(current_page_directory, virtual_address);

    if(frame_address) {
        pmm_mark_frame_available(frame_address);
    }
}

static void* vmm_find_free_memory(size_t size, bool is_kernel) {
    uint32_t space_base_address = is_kernel ? VMM_KERNEL_SPACE_BASE : VMM_USER_SPACE_BASE;
    uint32_t space_size = is_kernel ? VMM_KERNEL_SPACE_SIZE : VMM_USER_SPACE_SIZE;

    for(uint32_t page_address = space_base_address;
        page_address < space_base_address + space_size;
        page_address += PAGE_SIZE) {
        
        if(!paging_is_page_used(current_page_directory, (void*) page_address)) {
            bool is_free = true;

            for(uint32_t index = 0; index < size; index += PAGE_SIZE) {
                // Ensure memory region is still within memory space bounds
                if(page_address + index >= space_base_address + space_size) {
                    is_free = false;
                    break;
                }

                if(paging_is_page_used(current_page_directory, (void*) (page_address + index))) {
                    is_free = false;
                    break;
                }
            }

            if(is_free) {
                return (void*) page_address;
            }
        }
    }

    return NULL;
}

bool vmm_is_mapped(void* virtual_address) {
    return paging_is_page_used(current_page_directory, virtual_address);
}

void* vmm_get_physical_address(void* virtual_address) {
    return paging_virtual_to_physical_address(current_page_directory, virtual_address);
}

void vmm_switch_address_space(page_directory_t *page_directory) {
    paging_switch_page_directory(current_page_directory, page_directory);
    current_page_directory = page_directory;
}

page_directory_t* vmm_create_address_space() {
    return vmm_clone_address_space(kernel_page_directory);
}

page_directory_t* vmm_clone_address_space(page_directory_t *src_page_directory) {
    page_directory_t *dst_page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));

    if(!dst_page_directory) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    for(size_t directory_index = 0; directory_index < PAGE_DIRECTORY_SIZE; directory_index++) {
        // Check if the page table is present
        if(src_page_directory->tables[directory_index]) {
            // Link page table instead of copying it if it is the kernel page table
            if(src_page_directory->tables[directory_index] == kernel_page_directory->tables[directory_index]) {
                dst_page_directory->tables[directory_index] = src_page_directory->tables[directory_index];
                dst_page_directory->entries[directory_index] = src_page_directory->entries[directory_index];
            } else {
                page_table_t* src_page_table = src_page_directory->tables[directory_index];
                page_table_t* dst_page_table = (page_table_t*) kmalloc_a(sizeof(page_table_t));

                if(!dst_page_table) {
                    KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
                }

                for(size_t table_index = 0; table_index < PAGE_TABLE_SIZE; table_index++) {
                    // Check if page is used
                    if(src_page_table->entries[table_index].page_base) {
                        uint32_t src_virtual_address = (directory_index << 22) | (table_index << 12);
                        uint32_t dst_virtual_address = src_virtual_address;
                        uint32_t tmp_virtual_address = 0;

                        // Map a frame to the new page table

                        uint32_t dst_physical_address = (uint32_t) pmm_alloc_frame();

                        if(!dst_physical_address) {
                            KPANIC(KPANIC_PMM_OUT_OF_MEMORY_CODE, KPANIC_PMM_OUT_OF_MEMORY_MESSAGE, NULL);
                        }

                        paging_map_page(dst_page_directory, (void*) dst_virtual_address, (void*) dst_physical_address, false, true);

                        // Temporary map the new physical frame to copy the page content

                        tmp_virtual_address = vmm_map_page(NULL, (void*) dst_physical_address, false, true);

                        memcpy((void*) tmp_virtual_address, (void*) src_virtual_address, PAGE_SIZE);

                        vmm_unmap_page((void*) tmp_virtual_address);

                        // Update the page table entry

                        dst_page_table->entries[table_index].page_base = dst_physical_address >> 12;
                        dst_page_table->entries[table_index].present = src_page_table->entries[table_index].present;
                        dst_page_table->entries[table_index].read_write = src_page_table->entries[table_index].read_write;
                        dst_page_table->entries[table_index].user_supervisor = src_page_table->entries[table_index].user_supervisor;
                        dst_page_table->entries[table_index].write_through = src_page_table->entries[table_index].write_through;
                    }
                }
            }
        }
    }

    return dst_page_directory;
}
