#include <memory/vmm.h>
#include <memory/pmm.h>
#include <memory/paging.h>
#include <sys/kpanic.h>
#include <memory/kheap.h>

static page_directory_t *kernel_page_directory = NULL;
static page_directory_t *current_page_directory = NULL;

static int32_t vmm_map_page(void *const virtual_address, void* physical_address, bool is_kernel, bool is_writeable);
static void vmm_unmap_page(void *const virtual_address);
static void* vmm_find_free_memory(size_t size, bool is_kernel);

void vmm_init() {
    current_page_directory = prepaging_page_directory;

    kernel_page_directory = (page_directory_t*) kmalloc_a(sizeof(page_directory_t));

    if(!kernel_page_directory) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    memset(kernel_page_directory, 0, sizeof(page_directory_t));

    // Mapping lower memory + higher half kernel (Physical: 0x00000000 - 0x0FFFFFFF)
    for(uint32_t page_address = VMM_LOWER_MEMORY_BASE;
        page_address < VMM_LOWER_MEMORY_BASE + VMM_LOWER_MEMORY_SIZE + VMM_HIGHER_HALF_SIZE;
        page_address += PAGE_SIZE) {
        paging_allocate_page(kernel_page_directory, (void*) page_address, (void*) (page_address - VMM_LOWER_MEMORY_BASE), true, true);
        pmm_mark_frame_reserved((void*) (page_address - VMM_LOWER_MEMORY_BASE));
    }

    paging_switch_page_directory(current_page_directory, kernel_page_directory);

    current_page_directory = kernel_page_directory;

    paging_enable();
}

void* vmm_map_memory(void* virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable) {
    uint32_t frame_address = (uint32_t) physical_address;

    if(!virtual_address) {
        virtual_address = vmm_find_free_memory(size, is_kernel);

        if(!virtual_address) {
            return NULL;
        }
    }

    // Page align the virtual address
    virtual_address = VMM_ALIGN(virtual_address);

    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
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

static int32_t vmm_map_page(void *const virtual_address, void* physical_address, bool is_kernel, bool is_writeable) {
    // If no frame address is provided, allocate a new frame
    if(!physical_address) {
        physical_address = pmm_alloc_frame();

        if(!physical_address) {
            KPANIC(KPANIC_PMM_OUT_OF_MEMORY_CODE, KPANIC_PMM_OUT_OF_MEMORY_MESSAGE, NULL);
        }
    } else {
        pmm_mark_frame_reserved(physical_address);
    }

    return paging_allocate_page(current_page_directory, virtual_address, physical_address, is_kernel, is_writeable);
}

void vmm_unmap_memory(void *const virtual_address, size_t size) {
    for(uint32_t page_address = (uint32_t) virtual_address;
        page_address < (uint32_t) virtual_address + size;
        page_address += PAGE_SIZE) {
        
        paging_free_page(current_page_directory, (void*) page_address);
    }
}

static void vmm_unmap_page(void *const virtual_address) {
    void* frame_address = paging_free_page(current_page_directory, virtual_address);

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

void* vmm_get_mapped_address(void* virtual_address) {
    return paging_virtual_to_physical_address(current_page_directory, virtual_address);
}
