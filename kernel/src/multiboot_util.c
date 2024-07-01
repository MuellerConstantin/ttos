#include <multiboot_util.h>

size_t multiboot_get_memory_size(multiboot_info_t *multiboot_info) {
    if(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP) {
        size_t total_memory_size = 0;

        for(uint32_t offset = 0;
            offset < multiboot_info->mmap_length;
            offset += sizeof(multiboot_memory_map_t)) {
            multiboot_memory_map_t *mmap = (multiboot_memory_map_t *) (multiboot_info->mmap_addr + offset);

            if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                total_memory_size += mmap->len;
            }
        }

        return total_memory_size;
    }

    return 0;
}
