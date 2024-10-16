/**
 * @file idt.h
 * @brief Definitions for the Interrupt Descriptor Table (IDT).
 * 
 * The Interrupt Descriptor Table (IDT) is a data structure used by the x86 architecture to handle
 * hardware interrupts and processor exceptions. This file contains the definitions of the IDT and its
 * descriptors as well as the functions to initialize the IDT.
 */

#ifndef _KERNEL_ARCH_I386_IDT_H
#define _KERNEL_ARCH_I386_IDT_H

#include <stdint.h>

#define IDT_SIZE 256

// Masks

#define IDT_GATE_TYPE_MASK 0b00001111
#define IDT_PRIVILEGE_LEVEL_MASK 0b01100000
#define IDT_PRESENT_MASK 0b10000000

struct idt_gate_descriptor {
    /**
     * The lower 16 bits of the offset. The offset is a 32-bit value that tells
     * the linear address of the handler function.
     */
    uint16_t offset_low;

    /**
     * The segment selector. This is the segment where the handler function is
     * located.
     */
    uint16_t selector;

    uint8_t unused;

    /**
     * This is a 3-bit value that tells the type of the gate. The type is used to
     * determine the type of the handler function.
     */
    uint8_t gate_type : 4;

    uint8_t reserved : 1;

    uint8_t privilege_level : 2;

    /**
     * This is a 1-bit value that tells if the gate is present. If the gate is not
     * present, the CPU will raise an exception.
     */
    uint8_t present : 1;

    /**
     * The higher 16 bits of the offset. See offset_low for more information.
     */
    uint16_t offset_high;
} __attribute__((packed));

typedef struct idt_gate_descriptor idt_gate_descriptor_t;

/**
 * Structure representing the Interrupt Descriptor Table (IDT).
 */
struct idt_table {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct idt_table idt_table_t;

/**
 * Initializes the Interrupt Descriptor Table (IDT).
 */
void idt_init();

#endif // _KERNEL_ARCH_I386_IDT_H
