#include <memory/btalloc.h>

/**
 * The heap memory used for dynamic memory allocation during boot time.
 * This memory uses a shiftable pointer to allocate memory in a linear
 * fashion. This also means allocated memory cannot be freed because
 * nobody keeps track of it.
 * 
 * @note This memory is used only while the kernel heap is not
 * yet initialized.
 */
static uint8_t* btalloc_memory = NULL;
static uint8_t* btalloc_head = NULL;
static size_t btalloc_size = 0;

void btalloc_init(uint32_t memory_address, size_t size) {
    btalloc_memory = (uint8_t*) memory_address;
    btalloc_head = btalloc_memory;
    btalloc_size = size;
    memset(btalloc_memory, 0, size);
}

void* btalloc_malloc(size_t size, bool align) {
    if(!btalloc_head) {
        return NULL;
    }

    // Prevent memory allocation beyond the heap memory
    if(btalloc_head + size > btalloc_memory + btalloc_size) {
        return NULL;
    }

    void* address = btalloc_head;

    if(align && !BTALLOC_IS_ALIGN(address)) {
        void* aligned_address = (void *) BTALLOC_ALIGN(address);
        size_t offset = (size_t) aligned_address - (size_t) address;

        size += offset;
        address = aligned_address;
    }

    btalloc_head += size;

    return address;
}
