#ifndef _KERNEL_DESCRIPTORS_TSS_H
#define _KERNEL_DESCRIPTORS_TSS_H

#include <stdint.h>
#include <string.h>
#include <descriptors/gdt.h>

struct tss_segment_descriptor {
    uint16_t link;
    uint16_t reserved0;
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved4;
    uint16_t cs;
    uint16_t reserved5;
    uint16_t ss;
    uint16_t reserved6;
    uint16_t ds;
    uint16_t reserved7;
    uint16_t fs;
    uint16_t reserved8;
    uint16_t gs;
    uint16_t reserved9;
    uint16_t ldt;
    uint16_t reserved10;
    uint16_t reserved11;
    uint16_t iopb;
    uint32_t ssp;
} __attribute__ ((packed));

typedef struct tss_segment_descriptor tss_segment_descriptor_t;

/**
 * Initializes the Task State Segment (TSS).
 * 
 * @param ss0 The stack segment for ring 0 (kernel mode).
 * @param esp0 The stack pointer for ring 0 (kernel mode).
 */
void tss_init(uint16_t ss0, uintptr_t esp0);

/**
 * Updates the stack segment and stack pointer for ring 0 (kernel mode).
 * 
 * @param ss0 The stack segment for ring 0 (kernel mode).
 * @param esp0 The stack pointer for ring 0 (kernel mode).
 */
void tss_update_stack(uint16_t ss0, uintptr_t esp0);

#endif // _KERNEL_DESCRIPTORS_TSS_H