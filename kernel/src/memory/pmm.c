#include <memory/pmm.h>
#include <system/kpanic.h>
#include <memory/kheap.h>
#include <system/kmessage.h>
#include <stdio.h>

static linked_list_t* pmm_memory_regions = NULL;
static size_t pmm_total_memory_size = 0;
static size_t pmm_num_memory_frames = 0;
static size_t pmm_num_memory_frames_used = 0;
static size_t pmm_bitmap_size = 0;
static uint8_t *pmm_bitmap = NULL;

static linked_list_t* pmm_fetch_memory_regions(multiboot_info_t *multiboot_info);
static int pmm_memory_region_compare(void* a, void* b);
static size_t pmm_fetch_total_memory_size(linked_list_t* memory_regions);
static bool pmm_test_frame(uint32_t frame);
static void pmm_set_frame(uint32_t frame);
static void pmm_unset_frame(uint32_t frame);
static int pmm_find_free_frame();
static int pmm_find_free_contiguous_frames(size_t n);

void pmm_init(multiboot_info_t *multiboot_info) {
    pmm_memory_regions = pmm_fetch_memory_regions(multiboot_info);
    pmm_total_memory_size = pmm_fetch_total_memory_size(pmm_memory_regions);

    // Create bitmap for full physical address space
    pmm_num_memory_frames = PMM_PAS_SIZE / PMM_FRAME_SIZE;
    pmm_num_memory_frames_used = pmm_num_memory_frames;
    pmm_bitmap_size = ceil((double) pmm_num_memory_frames / (double) PMM_FRAMES_PER_BITMAP_BYTE);
    pmm_bitmap = (uint8_t *) kmalloc_a(pmm_bitmap_size);

    if(!pmm_bitmap) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    // By default, all memory frames are used
    memset(pmm_bitmap, 0xFF, pmm_bitmap_size);

    linked_list_foreach(pmm_memory_regions, node) {
        pmm_memory_region_t* region = (pmm_memory_region_t*) node->data;

        if(region->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint32_t region_start_frame = pmm_address_to_index((void*) region->base);
            uint32_t region_end_frame = pmm_address_to_index((void*) (region->base + region->length - 1));

            // Mark the memory region as available
            for(uint32_t frame = region_start_frame; frame <= region_end_frame; frame++) {
                pmm_unset_frame(frame);
            }
        }
    }

    char* kernel_message = kmalloc(96);

    if(!kernel_message) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message, "memory: PMM initialized with %d (%f MB) memory frames", pmm_num_memory_frames, pmm_total_memory_size / 1024.0 / 1024.0);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message);
}

static linked_list_t* pmm_fetch_memory_regions(multiboot_info_t *multiboot_info) {
    linked_list_t* memory_regions = linked_list_create();

    if(!memory_regions) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    // Create a linked list of memory regions
    for(uint32_t offset = 0;
        offset < multiboot_info->mmap_length;
        offset += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t *) (multiboot_info->mmap_addr + offset);
        pmm_memory_region_t* region = (pmm_memory_region_t*) kmalloc(sizeof(pmm_memory_region_t));

        if(!region) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        region->base = mmap->addr;
        region->length = mmap->len;
        region->type = mmap->type;

        linked_list_node_t* node = linked_list_create_node(region);

        if(!node) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        linked_list_append(memory_regions, node);
    }

    // Sort the memory regions by base address
    linked_list_sort(memory_regions, pmm_memory_region_compare);

    // Clean up overlapping memory regions
    for(linked_list_node_t* node = memory_regions->head; node != NULL; node = node->next) {
        for(linked_list_node_t* next = node->next; next != NULL; next = next->next) {
            pmm_memory_region_t* region = (pmm_memory_region_t*) node->data;
            pmm_memory_region_t* next_region = (pmm_memory_region_t*) next->data;

            // If the regions overlap, adjust the length of the region with the lower type
            if(region->base + region->length > next_region->base) {
                if(region->type < next_region->type) {
                    region->length = next_region->base - region->base;
                } else {
                    next_region->length = region->base - next_region->base;
                }
            }
        }
    }

    return memory_regions;
}

static int pmm_memory_region_compare(void* a, void* b) {
    pmm_memory_region_t* region_a = (pmm_memory_region_t*) a;
    pmm_memory_region_t* region_b = (pmm_memory_region_t*) b;

    // Sort first by base address, then by end address

    if(region_a->base < region_b->base) {
        return -1;
    } else if(region_a->base > region_b->base) {
        return 1;
    }

    if(region_a->length < region_b->length) {
        return -1;
    } else if(region_a->length > region_b->length) {
        return 1;
    }

    return 0;
}

static size_t pmm_fetch_total_memory_size(linked_list_t* memory_regions) {
    size_t installed_memory_size = 0;

    linked_list_foreach(memory_regions, node) {
        pmm_memory_region_t* region = (pmm_memory_region_t*) node->data;

        installed_memory_size += region->length;
    }

    return installed_memory_size;
}

const linked_list_t* pmm_get_memory_regions() {
    return pmm_memory_regions;
}

size_t pmm_get_total_memory_size() {
    return pmm_total_memory_size;
}

size_t pmm_get_available_memory_size() {
    return (pmm_num_memory_frames - pmm_num_memory_frames_used) * PMM_FRAME_SIZE;
}

uint32_t pmm_address_to_index(void* address) {
    return (uint32_t) address / PMM_FRAME_SIZE;
}

void* pmm_index_to_address(uint32_t index) {
    return (void *) (index * PMM_FRAME_SIZE);
}

static bool pmm_test_frame(uint32_t frame) {
    return pmm_bitmap[frame / PMM_FRAMES_PER_BITMAP_BYTE] & 1 << (frame % PMM_FRAMES_PER_BITMAP_BYTE);
}

static void pmm_set_frame(uint32_t frame) {
    if(pmm_test_frame(frame)) {
        return;
    }

    pmm_bitmap[frame / PMM_FRAMES_PER_BITMAP_BYTE] |= (1 << (frame % PMM_FRAMES_PER_BITMAP_BYTE));
    pmm_num_memory_frames_used++;
}

static void pmm_unset_frame(uint32_t frame) {
    if(!pmm_test_frame(frame)) {
        return;
    }

    pmm_bitmap[frame / PMM_FRAMES_PER_BITMAP_BYTE] &= ~(1 << (frame % PMM_FRAMES_PER_BITMAP_BYTE));
    pmm_num_memory_frames_used--;
}

void pmm_mark_frame_reserved(void* frame_addr) {
    uint32_t frame = pmm_address_to_index(frame_addr);

    pmm_set_frame(frame);
}

void pmm_mark_frame_available(void* frame_addr) {
    uint32_t frame = pmm_address_to_index(frame_addr);

    pmm_unset_frame(frame);
}

static int pmm_find_free_frame() {
    for (size_t index = 0; index < pmm_num_memory_frames / PMM_FRAMES_PER_BITMAP_BYTE; index++) {
        if (pmm_bitmap[index] != 0xFF) {
            for (int bit_index = 0; bit_index < PMM_FRAMES_PER_BITMAP_BYTE; bit_index++) {
                if (!(pmm_bitmap[index] & (1 << bit_index))) {
                    return index * PMM_FRAMES_PER_BITMAP_BYTE + bit_index;
                }
            }
        }
    }

    return -1;
}

static int pmm_find_free_contiguous_frames(size_t n) {
    size_t free_frames = 0;
    size_t start_frame = 0;

    for (size_t index = 0; index < pmm_num_memory_frames / PMM_FRAMES_PER_BITMAP_BYTE; index++) {
        if (pmm_bitmap[index] != 0xFF) {
            for (int bit_index = 0; bit_index < PMM_FRAMES_PER_BITMAP_BYTE; bit_index++) {
                if (!(pmm_bitmap[bit_index] & (1 << bit_index))) {
                    if (free_frames == 0) {
                        start_frame = index * PMM_FRAMES_PER_BITMAP_BYTE + bit_index;
                    }

                    free_frames++;

                    if (free_frames == n) {
                        return start_frame;
                    }
                } else {
                    free_frames = 0;
                }
            }
        }
    }

    return -1;
}

void* pmm_alloc_frame() {
    if(pmm_num_memory_frames_used >= pmm_num_memory_frames) {
        return NULL;
    }

    int frame = pmm_find_free_frame();

    if(frame == -1) {
        return NULL;
    }

    pmm_set_frame(frame);

    return pmm_index_to_address(frame);
}

void* pmm_alloc_frames(size_t n) {
    if(pmm_num_memory_frames_used + n >= pmm_num_memory_frames) {
        return NULL;
    }

    int frame = pmm_find_free_contiguous_frames(n);

    if(frame == -1) {
        return NULL;
    }

    for(size_t i = 0; i < n; i++) {
        pmm_set_frame(frame + i);
    }

    return pmm_index_to_address(frame);
}

void pmm_free_frame(void *frame_addr) {
    int frame = pmm_address_to_index(frame_addr);

    pmm_unset_frame(frame);
}

void pmm_free_frames(void *frame_addr, size_t n) {
    int frame = pmm_address_to_index(frame_addr);

    for(size_t i = 0; i < n; i++) {
        pmm_unset_frame(frame + i);
    }
}
