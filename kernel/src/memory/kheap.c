#include <memory/kheap.h>

/**
 * The placement memory (limited to 1MB) that is used as a fallback
 * for the kernel heap when it is not yet initialized. This memory
 * is used for dynamic memory allocation during kernel startup. It
 * uses a shiftable pointer to allocate memory in a linear fashion.
 * This also means allocated memory cannot be freed because nobody
 * keeps track of it.
 * 
 * @note This memory is used only while the kernel heap is not
 * yet initialized.
 */
static const uint8_t* kheap_placement_memory = (uint8_t*) &kernel_virtual_end;
static uint8_t* kheap_placement_memory_head = (uint8_t*) &kernel_virtual_end;

static void* kmalloc_int(size_t size, bool align, uint32_t* physical_address);

void* kmalloc_a(uint32_t size) {
    return kmalloc_int(size, true, NULL);
}

void* kmalloc(uint32_t size) {
    return kmalloc_int(size, false, NULL);
}

static void* kmalloc_int(size_t size, bool align, uint32_t* physical_address) {
    // Prevent memory allocation beyond the heap placement memory
    if(kheap_placement_memory_head + size > kheap_placement_memory + KHEAP_PLACEMENT_MEMORY_SIZE) {
        return NULL;
    }

    void* address = kheap_placement_memory_head;

    if(align && !KHEAP_IS_ALIGNED(address)) {
        void* aligned_address = (void *) KHEAP_ALIGN(address);
        size_t offset = (size_t) aligned_address - (size_t) address;

        size += offset;
        address = aligned_address;
    }

    kheap_placement_memory_head += size;

    if(physical_address) {
        *physical_address = (uint32_t) address;
    }

    return address;
}
