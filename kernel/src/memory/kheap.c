#include <memory/kheap.h>
#include <memory/paging.h>

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
static uint8_t kheap_placement_memory[KHEAP_PLACEMENT_SIZE];
static uint8_t* kheap_placement_memory_head = kheap_placement_memory;

static bool kheap_enabled = false;

/**
 * Begin of memory allocated for the kernel heap. This is the first
 * address that can be used for dynamic memory allocation.
 * 
 * @note As soon as the kernel heap is initialized, this pointer
 * will be set to the start of the heap.
 */
static kheap_block_t* kheap_head = NULL;
static kheap_block_t* kheap_tail = NULL;

static void* kmalloc_int(size_t size, bool align);
static void* kmalloc_heap(size_t size, bool align);
static void* kmalloc_placement(size_t size, bool align);
static void* kheap_find_best_fit(size_t size, bool align);
static inline bool kheap_is_valid_heap_address(void* ptr);

void kheap_init() {
    // Mapping the kernel heap's virtual address space
    paging_map_memory((void*) KHEAP_HEAP_BASE, KHEAP_HEAP_SIZE, NULL, true, true);

    kheap_head = (kheap_block_t*) KHEAP_HEAP_BASE;
    kheap_head->prev = NULL;
    kheap_head->next = NULL;
    kheap_head->magic = KHEAP_MAGIC;
    kheap_head->size = KHEAP_HEAP_SIZE - sizeof(kheap_block_t);
    kheap_head->free = true;

    kheap_tail = kheap_head;

    kheap_enabled = true;
}

size_t kheap_get_total_memory_size() {
    return KHEAP_HEAP_SIZE;
}

size_t kheap_get_available_memory_size() {
    size_t free_memory = 0;

    kheap_block_t* current = kheap_head;

    while(current != NULL) {
        if(current->free) {
            free_memory += current->size;
        }

        current = current->next;
    }

    return free_memory;
}

void* kmalloc_a(size_t size) {
    return kmalloc_int(size, true);
}

void* kmalloc(size_t size) {
    return kmalloc_int(size, false);
}

void* kcalloc(size_t num, size_t size) {
    void* ptr = kmalloc(num * size);

    if(ptr != NULL) {
        memset(ptr, 0, num * size);
    }

    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if(ptr == NULL) {
        return kmalloc(size);
    }

    if(size == 0) {
        kfree(ptr);
        return NULL;
    }

    if(!kheap_is_valid_heap_address(ptr)) {
        return NULL;
    }

    kheap_block_t* block = (kheap_block_t*) ((uintptr_t) ptr - sizeof(kheap_block_t));

    if(block->size >= size) {
        return ptr;
    }

    void* new_ptr = kmalloc(size);

    if(new_ptr == NULL) {
        return NULL;
    }

    memcpy(new_ptr, ptr, block->size);

    kfree(ptr);

    return new_ptr;
}

static void* kmalloc_int(size_t size, bool align) {
    if(kheap_enabled) {
        // If the kernel heap is initialized, use it for memory allocation
        return kmalloc_heap(size, align);
    } else {
        // Otherwise, use the temporary placement memory
        return kmalloc_placement(size, align);
    }
}

static void* kmalloc_heap(size_t size, bool align) {
    if(size == 0) {
        return NULL;
    }

    size_t total_size = size + sizeof(kheap_block_t);

    kheap_block_t* best_fit = kheap_find_best_fit(total_size, align);

    if(best_fit == NULL) {
        return NULL;
    }

    if(align) {
        uintptr_t block_addr = (uintptr_t) best_fit + sizeof(kheap_block_t);
        uintptr_t aligned_addr = (block_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        size_t alignment_padding = aligned_addr - block_addr;

        // Check if block is already aligned
        if(alignment_padding > 0) {
            // Ensure block has enough space for alignment padding block and can be split
            if(best_fit->size > total_size + alignment_padding + sizeof(kheap_block_t)) {
                kheap_block_t* new_best_fit = (kheap_block_t*) ((uintptr_t) best_fit + alignment_padding + sizeof(kheap_block_t));
                new_best_fit->free = true;
                new_best_fit->magic = KHEAP_MAGIC;
                new_best_fit->size = best_fit->size - alignment_padding - sizeof(kheap_block_t);
                new_best_fit->next = best_fit->next;
                new_best_fit->prev = best_fit;

                if(best_fit->next != NULL) {
                    best_fit->next->prev = new_best_fit;
                } else {
                    kheap_tail = new_best_fit;
                }

                // The address of the original best fit is now the address of the padding block
                kheap_block_t* padding_block = best_fit;

                padding_block->free = true;
                padding_block->size = alignment_padding;
                padding_block->next = new_best_fit;

                best_fit = new_best_fit;
            } else {
                return NULL;
            }
        }
    }

    size_t remaining_size = best_fit->size - total_size;

    best_fit->free = false;

    // Split the block if there is enough space for a new block
    if(remaining_size > sizeof(kheap_block_t)) {
        kheap_block_t* remaining_block = (kheap_block_t*) ((uintptr_t) best_fit + total_size);
        remaining_block->free = true;
        remaining_block->magic = KHEAP_MAGIC;
        remaining_block->size = remaining_size - sizeof(kheap_block_t);
        remaining_block->next = best_fit->next;
        remaining_block->prev = best_fit;

        if(best_fit->next != NULL) {
            best_fit->next->prev = remaining_block;
        } else {
            kheap_tail = remaining_block;
        }

        best_fit->size = size;
        best_fit->next = remaining_block;
    }

    return (void*) ((uintptr_t) best_fit + sizeof(kheap_block_t));
}

static void* kheap_find_best_fit(size_t size, bool align) {
    kheap_block_t* current = kheap_head;
    kheap_block_t* best_fit = NULL;

    while(current != NULL) {
        if (current->free && current->size >= size) {
            if (align) {
                uintptr_t block_addr = (uintptr_t) current + sizeof(kheap_block_t);
                uintptr_t aligned_addr = (block_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
                size_t alignment_padding = aligned_addr - block_addr;

                // Ensure that the block has enough additional space for alignment padding
                if (current->size >= size + alignment_padding + sizeof(kheap_block_t)) {
                    if (best_fit == NULL || current->size < best_fit->size) {
                        best_fit = current;
                    }
                }
            } else {
                if (best_fit == NULL || current->size < best_fit->size) {
                    best_fit = current;
                }
            }
        }

        current = current->next;
    }

    return best_fit;
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    if (!kheap_is_valid_heap_address(ptr)) {
        return;
    }

    kheap_block_t* block = (kheap_block_t*) ((uintptr_t) ptr - sizeof(kheap_block_t));

    block->free = true;

    // Merge with previous block if it is free
    if (block->prev != NULL && block->prev->magic == KHEAP_MAGIC && block->prev->free) {
        block->prev->size += block->size + sizeof(kheap_block_t);
        block->prev->next = block->next;

        if (block->next != NULL) {
            block->next->prev = block->prev;
        } else {
            kheap_tail = block->prev;
        }

        block = block->prev;

        memset((void*) ((uintptr_t) block + sizeof(kheap_block_t)), 0, block->size);
    }

    // Merge with next block if it is free
    if (block->next != NULL && block->next->magic == KHEAP_MAGIC && block->next->free) {
        block->size += block->next->size + sizeof(kheap_block_t);
        block->next = block->next->next;

        if (block->next != NULL) {
            block->next->prev = block;
        } else {
            kheap_tail = block;
        }

        memset((void*) ((uintptr_t) block + sizeof(kheap_block_t)), 0, block->size);
    }
}

static inline bool kheap_is_valid_heap_address(void* ptr) {
    if(!kheap_enabled) {
        return false;
    }

    uintptr_t addr = (uintptr_t) ptr;

    bool in_space = addr >= (uintptr_t) KHEAP_HEAP_BASE + sizeof(kheap_block_t) && addr < (uintptr_t) KHEAP_HEAP_BASE + KHEAP_HEAP_SIZE;

    kheap_block_t* block = (kheap_block_t*) ((uintptr_t) ptr - sizeof(kheap_block_t));

    bool valid_block = block->magic == KHEAP_MAGIC;

    return in_space && valid_block;
}

static void* kmalloc_placement(size_t size, bool align) {
    void* addr = kheap_placement_memory_head;

    if(align) {
        uintptr_t aligned_addr = ((uintptr_t) addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        size_t alignment_padding = aligned_addr - (uintptr_t) addr;

        size += alignment_padding;
        addr = (void*) aligned_addr;
    }

    // Prevent memory allocation beyond the heap placement memory
    if(kheap_placement_memory_head + size > kheap_placement_memory + KHEAP_PLACEMENT_SIZE) {
        return NULL;
    }

    kheap_placement_memory_head += size;

    return addr;
}
