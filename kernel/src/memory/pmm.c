#include <memory/pmm.h>

static size_t pmm_memory_size = 0;
static size_t pmm_num_memory_frames = 0;
static size_t pmm_num_memory_frames_used = 0;
static uint8_t *pmm_bitmap = 0;

static bool pmm_test_frame(int frame);
static void pmm_set_frame(int frame);
static void pmm_unset_frame(int frame);

void pmm_init(size_t memory_size, uint32_t bitmap_addr) {
    pmm_memory_size = memory_size;
    pmm_num_memory_frames = memory_size / PMM_BLOCK_SIZE;
    pmm_num_memory_frames_used = pmm_num_memory_frames;
    pmm_bitmap = (uint8_t *) bitmap_addr;

    memset(pmm_bitmap, 0xFF, pmm_num_memory_frames / PMM_BLOCKS_PER_BITMAP_BYTE);
}

size_t pmm_get_free_memory_size() {
    return (pmm_num_memory_frames - pmm_num_memory_frames_used) * PMM_BLOCK_SIZE;
}

size_t pmm_get_total_memory_size() {
    return pmm_memory_size;
}

void pmm_mark_region_available(uint32_t base, size_t size) {
    int align = base / PMM_BLOCK_SIZE;
    int blocks = ceil((double) size / (double) PMM_BLOCK_SIZE);

    for (; blocks > 0; blocks--) {
        pmm_unset_frame(align++);
        pmm_num_memory_frames_used--;
    }

    pmm_set_frame(0);
}

void pmm_mark_region_reserved(uint32_t base, size_t size) {
    int align = base / PMM_BLOCK_SIZE;
    int blocks = ceil((double) size / (double) PMM_BLOCK_SIZE);

    for (; blocks > 0; blocks--) {
        pmm_set_frame(align++);
        pmm_num_memory_frames_used++;
    }

    pmm_set_frame(0);
}

static bool pmm_test_frame(int frame) {
    return pmm_bitmap[frame / PMM_BLOCKS_PER_BITMAP_BYTE] & 1 << (frame % PMM_BLOCKS_PER_BITMAP_BYTE);
}

static void pmm_set_frame(int frame) {
    pmm_bitmap[frame / PMM_BLOCKS_PER_BITMAP_BYTE] |= ~(1 << (frame % PMM_BLOCKS_PER_BITMAP_BYTE));
}

static void pmm_unset_frame(int frame) {
    pmm_bitmap[frame / PMM_BLOCKS_PER_BITMAP_BYTE] &= ~(1 << (frame % PMM_BLOCKS_PER_BITMAP_BYTE));
}

static int pmm_find_free_frame() {
    for (size_t index = 0; index < pmm_num_memory_frames / PMM_BLOCKS_PER_BITMAP_BYTE; index++) {
        if (pmm_bitmap[index] != 0xFF) {
            for (int bit_index = 0; bit_index < PMM_BLOCKS_PER_BITMAP_BYTE; bit_index++) {
                if (!(pmm_bitmap[bit_index] & (1 << bit_index))) {
                    return index * PMM_BLOCKS_PER_BITMAP_BYTE + bit_index;
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

    return (void *) (frame * PMM_BLOCK_SIZE);
}

void pmm_free_frame(void *frame_addr) {
    int frame = (int) frame_addr / PMM_BLOCK_SIZE;

    pmm_unset_frame(frame);
    pmm_num_memory_frames_used--;
}
