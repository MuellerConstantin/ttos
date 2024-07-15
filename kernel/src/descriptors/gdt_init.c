#include <descriptors/gdt.h>

static gdt_table_t gdt;
static gdt_segment_descriptor_t descriptors[GDT_SIZE];

extern void gdt_flush(uint32_t gdt_ptr);

void gdt_init() {
    for(int selector = 0; selector < GDT_SIZE; selector++) {
        descriptors[selector].limit_low = 0;
        descriptors[selector].base_low = 0;
        descriptors[selector].base_middle = 0;

        descriptors[selector].accessed = 0;
        descriptors[selector].read_write = 0;
        descriptors[selector].direction_conforming = 0;
        descriptors[selector].executable = 0;
        descriptors[selector].descriptor_type = 0;
        descriptors[selector].privilege_level = 0;
        descriptors[selector].present = 0;

        descriptors[selector].limit_high = 0;
        descriptors[selector].reserved = 0;
        descriptors[selector].long_mode = 0;
        descriptors[selector].size = 0;
        descriptors[selector].granularity = 0;

        descriptors[selector].base_high = 0;
    }

    gdt_init_descriptor(GDT_NULL_SELEKTOR, 0, 0, 0, 0);

    gdt_init_descriptor(GDT_KERNEL_CODE_SELEKTOR, 0x00000000, 0x000FFFFF,
        GDT_ACCESS_PRESENT|GDT_ACCESS_RING0|GDT_ACCESS_STANDARD_SEGMENT|GDT_ACCESS_EXECUTABLE|GDT_ACCESS_READWRITE,
        GDT_FLAGS_GRANULARITY_4KB|GDT_FLAGS_BIT_32_SEGMENT);
    
    gdt_init_descriptor(GDT_KERNEL_DATA_SELEKTOR, 0x00000000, 0x000FFFFF,
        GDT_ACCESS_PRESENT|GDT_ACCESS_RING0|GDT_ACCESS_STANDARD_SEGMENT|GDT_ACCESS_DIRECTION_UP|GDT_ACCESS_READWRITE,
        GDT_FLAGS_GRANULARITY_4KB|GDT_FLAGS_BIT_32_SEGMENT);

    gdt_init_descriptor(GDT_USER_CODE_SELEKTOR, 0x00000000, 0x000FFFFF,
        GDT_ACCESS_PRESENT|GDT_ACCESS_RING3|GDT_ACCESS_STANDARD_SEGMENT|GDT_ACCESS_EXECUTABLE|GDT_ACCESS_READWRITE,
        GDT_FLAGS_GRANULARITY_4KB|GDT_FLAGS_BIT_32_SEGMENT);

    gdt_init_descriptor(GDT_USER_DATA_SELEKTOR, 0x00000000, 0x000FFFFF,
        GDT_ACCESS_PRESENT|GDT_ACCESS_RING3|GDT_ACCESS_STANDARD_SEGMENT|GDT_ACCESS_DIRECTION_UP|GDT_ACCESS_READWRITE,
        GDT_FLAGS_GRANULARITY_4KB|GDT_FLAGS_BIT_32_SEGMENT);

    gdt.base = (uint32_t) &descriptors;
    gdt.limit = sizeof(descriptors) - 1;

    gdt_flush((uint32_t) &gdt);
}

void gdt_init_descriptor(gdt_selector_t selector, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    // Setup fractional base address (segment's start address)
    descriptors[selector].base_low = (base & 0xFFFF);
    descriptors[selector].base_middle = (base >> 16) & 0xFF;
    descriptors[selector].base_high  = (base >> 24) & 0xFF;

    // Setup fractional limit (segment's size)
    descriptors[selector].limit_low = (limit & 0xFFFF);
    descriptors[selector].limit_high = (limit >> 16) & 0x0F;

    // Setup access flags
    descriptors[selector].accessed = (access & GDT_ACCESS_ACCESSED_MASK) >> 0;
    descriptors[selector].read_write = (access & GDT_ACCESS_READWRITE_MASK) >> 1;
    descriptors[selector].direction_conforming = (access & GDT_ACCESS_DIRECTION_CONFORMING_MASK) >> 2;
    descriptors[selector].executable = (access & GDT_ACCESS_EXECUTABLE_MASK) >> 3;
    descriptors[selector].descriptor_type = (access & GDT_ACCESS_DESCRIPTOR_TYPE_MASK) >> 4;
    descriptors[selector].privilege_level = (access & GDT_ACCESS_PRIVILEGE_LEVEL_MASK) >> 5;
    descriptors[selector].present = (access & GDT_ACCESS_PRESENT_MASK) >> 7;

    // Setup flags
    descriptors[selector].reserved = 0;
    descriptors[selector].long_mode = (flags & GDT_FLAGS_LONG_MODE_MASK) >> 5;
    descriptors[selector].size = (flags & GDT_FLAGS_SIZE_MASK) >> 6;
    descriptors[selector].granularity = (flags & GDT_FLAGS_GRANULARITY_MASK) >> 7;
}
