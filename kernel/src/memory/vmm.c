#include <memory/vmm.h>
#include <sys/kpanic.h>
#include <memory/kheap.h>

static page_directory_t *kernel_page_directory = NULL;
static page_directory_t *current_page_directory = NULL;

static int32_t vmm_map_page(void *const virtual_address, void* physical_address, bool is_kernel, bool is_writeable);
static void vmm_unmap_page(void *const virtual_address);

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
        pmm_mark_region_reserved((void*) (page_address - VMM_LOWER_MEMORY_BASE), PAGE_SIZE);
    }

    paging_switch_page_directory(current_page_directory, kernel_page_directory);

    current_page_directory = kernel_page_directory;

    paging_enable();
}

int32_t vmm_map_memory(void *const virtual_address, size_t size, void* physical_address, bool is_kernel, bool is_writeable) {
    uint32_t frame_address = (uint32_t) physical_address;

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

    return 0;
}

static int32_t vmm_map_page(void *const virtual_address, void* physical_address, bool is_kernel, bool is_writeable) {
    // If no frame address is provided, allocate a new frame
    if(!physical_address) {
        physical_address = pmm_alloc_frame();

        if(!physical_address) {
            KPANIC(KPANIC_PMM_OUT_OF_MEMORY_CODE, KPANIC_PMM_OUT_OF_MEMORY_MESSAGE, NULL);
        }
    } else {
        pmm_mark_region_reserved(physical_address, PAGE_SIZE);
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
        pmm_free_frame(frame_address);
    }
}
