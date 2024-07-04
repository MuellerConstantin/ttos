#include <memory/pmm.h>
#include <memory/kheap.h>
#include <drivers/serial/uart/16550.h>

static size_t pmm_memory_size = 0;
static size_t pmm_num_memory_frames = 0;
static size_t pmm_num_memory_frames_used = 0;
static size_t pmm_bitmap_size = 0;
static uint8_t *pmm_bitmap = NULL;

static bool pmm_test_frame(uint32_t frame);
static void pmm_set_frame(uint32_t frame);
static void pmm_unset_frame(uint32_t frame);
static int pmm_find_free_frame();
static int pmm_find_free_contiguous_frames(size_t n);

void pmm_init(size_t memory_size) {
    pmm_memory_size = memory_size;
    pmm_num_memory_frames = memory_size / PMM_FRAME_SIZE;
    pmm_num_memory_frames_used = 0;
    pmm_bitmap_size = ceil((double) pmm_num_memory_frames / (double) PMM_FRAMES_PER_BITMAP_BYTE);
    pmm_bitmap = (uint8_t *) kmalloc_a(pmm_bitmap_size);

    memset(pmm_bitmap, 0x00, pmm_bitmap_size);
}

size_t pmm_get_available_memory_size() {
    return (pmm_num_memory_frames - pmm_num_memory_frames_used) * PMM_FRAME_SIZE;
}

size_t pmm_get_total_memory_size() {
    return pmm_memory_size;
}

void pmm_mark_region_available(void* base, size_t size) {
    int align = (uint32_t) base / PMM_FRAME_SIZE;
    int frames = ceil((double) size / (double) PMM_FRAME_SIZE);

    for (; frames > 0; frames--) {
        pmm_unset_frame(align++);
        pmm_num_memory_frames_used--;
    }
}

void pmm_mark_region_reserved(void* base, size_t size) {
    int align = (uint32_t) base / PMM_FRAME_SIZE;
    int frames = ceil((double) size / (double) PMM_FRAME_SIZE);

    for (; frames > 0; frames--) {
        pmm_set_frame(align++);
        pmm_num_memory_frames_used++;
    }
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
    pmm_bitmap[frame / PMM_FRAMES_PER_BITMAP_BYTE] |= ~(1 << (frame % PMM_FRAMES_PER_BITMAP_BYTE));
}

static void pmm_unset_frame(uint32_t frame) {
    pmm_bitmap[frame / PMM_FRAMES_PER_BITMAP_BYTE] &= ~(1 << (frame % PMM_FRAMES_PER_BITMAP_BYTE));
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
    pmm_num_memory_frames_used++;

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

    pmm_num_memory_frames_used += n;

    return pmm_index_to_address(frame);
}

void pmm_free_frame(void *frame_addr) {
    int frame = (int) frame_addr / PMM_FRAME_SIZE;

    pmm_unset_frame(frame);
    pmm_num_memory_frames_used--;
}

void pmm_free_frames(void *frame_addr, size_t n) {
    int frame = (int) frame_addr / PMM_FRAME_SIZE;

    for(size_t i = 0; i < n; i++) {
        pmm_unset_frame(frame + i);
    }

    pmm_num_memory_frames_used -= n;
}
