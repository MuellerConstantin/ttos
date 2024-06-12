#ifndef _KERNEL_ARCH_X86_SYS_GDT_H
#define _KERNEL_ARCH_X86_SYS_GDT_H

#include <stdint.h>

#define GDT_SIZE 10

// Masks

#define GDT_FLAGS_GRANULARITY_MASK 0b10000000
#define GDT_FLAGS_SIZE_MASK 0b01000000
#define GDT_FLAGS_LONG_MODE_MASK 0b00100000

#define GDT_ACCESS_PRESENT_MASK 0b10000000
#define GDT_ACCESS_PRIVILEGE_LEVEL_MASK 0b01100000
#define GDT_ACCESS_DESCRIPTOR_TYPE_MASK 0b00010000
#define GDT_ACCESS_EXECUTABLE_MASK 0b00001000
#define GDT_ACCESS_DIRECTION_CONFORMING_MASK 0b00000100
#define GDT_ACCESS_READWRITE_MASK 0b00000010
#define GDT_ACCESS_ACCESSED_MASK 0b00000001

// Flags

#define GDT_FLAGS_GRANULARITY_1B 0b00000000
#define GDT_FLAGS_GRANULARITY_4KB 0b10000000
#define GDT_FLAGS_BIT_16_SEGMENT 0b00000000
#define GDT_FLAGS_BIT_32_SEGMENT 0b01000000
#define GDT_FLAGS_BIT_64_SEGMENT 0b00100000

#define GDT_ACCESS_PRESENT 0b10000000
#define GDT_ACCESS_RING0 0b00000000
#define GDT_ACCESS_RING1 0b00100000
#define GDT_ACCESS_RING2 0b01000000
#define GDT_ACCESS_RING3 0b01100000
#define GDT_ACCESS_SYSTEM_SEGMENT 0b00000000
#define GDT_ACCESS_STANDARD_SEGMENT 0b00010000
#define GDT_ACCESS_EXECUTABLE 0b00001000
#define GDT_ACCESS_DIRECTION_UP 0b00000000
#define GDT_ACCESS_DIRECTION_DOWN 0b00000100
#define GD_ACCESS_CONFORMING 0b00000100
#define GDT_ACCESS_READWRITE 0b00000010
#define GDT_ACCESS_ACCESSED 0b00000001

struct gdt_segment_descriptor {
    /**
     * The lower 16 bits of the segment's limit. The limit is a 20-bit value that
     * tells the maximum addressable unit of the segment, either in 1 byte units
     * or in 4KiB pages.
     */
    uint16_t limit_low;

    /**
     * The lower 16 bits of the base address. The base address is A 32-bit value
     * containing the linear address where the segment begins.
     */
    uint16_t base_low;

    /**
     * The middle 8 bits of the base address. See base_low for more information.
     */
    uint8_t base_middle;

    uint8_t accessed : 1;
    uint8_t read_write : 1;
    uint8_t direction_conforming : 1;
    uint8_t executable : 1;
    uint8_t descriptor_type : 1;
    uint8_t privilege_level : 2;
    uint8_t present : 1;

    /**
     * The higher 4 bits of the limit. See limit_low for more information.
     */
    uint8_t limit_high : 4;


    uint8_t reserved : 1;
    uint8_t long_mode : 1;
    uint8_t size : 1;
    uint8_t granularity : 1;

    /**
     * The higher 8 bits of the base address. See base_low for more information.
     */
    uint8_t base_high;
} __attribute__ ((packed));

typedef struct gdt_segment_descriptor gdt_segment_descriptor_t;

/**
 * Structure representing the Global Descriptor Table (GDT).
 */
struct gdt_table {
	uint16_t limit;
	uint32_t base;
} __attribute__ ((packed));

typedef struct gdt_table gdt_table_t;

typedef enum {
    NULL_SELEKTOR,          // Reserved segment (Offset 0x00)
    KERNEL_CODE_SELEKTOR,   // Kernel Code Segment (Offset 0x08)
    KERNEL_DATA_SELEKTOR,   // Kernel Data Segment Offset 0x10
    USER_CODE_SELEKTOR,     // User Code Segment (Offset 0x18)
    USER_DATA_SELEKTOR,     // User Data Segment (Offset 0x20)
    TASK_STATE_SELEKTOR     // Task State Segment (Offset 0x28)
} gdt_selector_t;

/**
 * Initializes the Global Descriptor Table (GDT).
 */
void gdt_init();

#endif // _KERNEL_ARCH_X86_SYS_GDT_H
