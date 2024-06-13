#include <sys/idt.h>
#include <sys/isr.h>
#include <sys/interrupts.h>

static idt_table_t idt;
static idt_gate_descriptor_t descriptors[IDT_SIZE];

static void idt_init_descriptor(interrupt_t index, uint32_t offset, uint16_t selector, uint8_t flags);

extern void idt_flush(uint32_t idt_ptr);

void idt_init() {
    for(int index = 0; index < IDT_SIZE; index++) {
        descriptors[index].offset_low = 0;
        descriptors[index].selector = 0;
        descriptors[index].unused = 0;
        descriptors[index].gate_type = 0;
        descriptors[index].reserved = 0;
        descriptors[index].privilege_level = 0;
        descriptors[index].present = 0;
        descriptors[index].offset_high = 0;
    }

    idt_init_descriptor(DIVISION_BY_ZERO_EXCEPTION, (uint32_t) exc0, 0x08, 0x8E);
    idt_init_descriptor(DEBUG_EXCEPTION, (uint32_t) exc1, 0x08, 0x8E);
    idt_init_descriptor(NON_MASKABLE_INTERRUPT_EXCEPTION, (uint32_t) exc2, 0x08, 0x8E);
    idt_init_descriptor(BREAKPOINT_EXCEPTION, (uint32_t) exc3, 0x08, 0x8E);
    idt_init_descriptor(OVERFLOW_EXCEPTION, (uint32_t) exc4, 0x08, 0x8E);
    idt_init_descriptor(BOUND_RANGE_EXCEEDED_EXCEPTION, (uint32_t) exc5, 0x08, 0x8E);
    idt_init_descriptor(INVALID_OPCODE_EXCEPTION, (uint32_t) exc6, 0x08, 0x8E);
    idt_init_descriptor(DEVICE_NOT_AVAILABLE_EXCEPTION, (uint32_t) exc7, 0x08, 0x8E);
    idt_init_descriptor(DOUBLE_FAULT_EXCEPTION, (uint32_t) exc8, 0x08, 0x8E);
    idt_init_descriptor(COPROCESSOR_SEGMENT_OVERRUN_EXCEPTION, (uint32_t) exc9, 0x08, 0x8E);
    idt_init_descriptor(INVALID_TSS_EXCEPTION, (uint32_t) exc10, 0x08, 0x8E);
    idt_init_descriptor(SEGMENT_NOT_PRESENT_EXCEPTION, (uint32_t) exc11, 0x08, 0x8E);
    idt_init_descriptor(STACK_SEGMENT_FAULT_EXCEPTION, (uint32_t) exc12, 0x08, 0x8E);
    idt_init_descriptor(GENERAL_PROTECTION_EXCEPTION, (uint32_t) exc13, 0x08, 0x8E);
    idt_init_descriptor(PAGE_FAULT_EXCEPTION, (uint32_t) exc14, 0x08, 0x8E);
    idt_init_descriptor(RESERVED_EXCEPTION, (uint32_t) exc15, 0x08, 0x8E);
    idt_init_descriptor(FLOATING_POINT_EXCEPTION, (uint32_t) exc16, 0x08, 0x8E);
    idt_init_descriptor(ALIGNMENT_CHECK_EXCEPTION, (uint32_t) exc17, 0x08, 0x8E);
    idt_init_descriptor(MACHINE_CHECK_EXCEPTION, (uint32_t) exc18, 0x08, 0x8E);
    idt_init_descriptor(SIMD_FLOATING_POINT_EXCEPTION, (uint32_t) exc19, 0x08, 0x8E);

    idt.base = (uint32_t) &descriptors;
    idt.limit = sizeof(descriptors) - 1;

    idt_flush((uint32_t) &idt);
}

static void idt_init_descriptor(interrupt_t index, uint32_t offset, uint16_t selector, uint8_t flags) {
    descriptors[index].offset_low = offset & 0xFFFF;
    descriptors[index].offset_high = (offset >> 16) & 0xFFFF;
    descriptors[index].selector = selector;

    descriptors[index].gate_type = flags & IDT_GATE_TYPE_MASK;
    descriptors[index].privilege_level = (flags & IDT_PRIVILEGE_LEVEL_MASK) >> 5;
    descriptors[index].present = (flags & IDT_PRESENT_MASK) >> 7;
}
