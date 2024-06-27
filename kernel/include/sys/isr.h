#ifndef _KERNEL_SYS_ISR_H
#define _KERNEL_SYS_ISR_H

#include <stdint.h>

extern void exc0();
extern void exc1();
extern void exc2();
extern void exc3();
extern void exc4();
extern void exc5();
extern void exc6();
extern void exc7();
extern void exc8();
extern void exc9();
extern void exc10();
extern void exc11();
extern void exc12();
extern void exc13();
extern void exc14();
extern void exc15();
extern void exc16();
extern void exc17();
extern void exc18();
extern void exc19();
extern void exc20();
extern void exc21();
extern void exc22();
extern void exc23();
extern void exc24();
extern void exc25();
extern void exc26();
extern void exc27();
extern void exc28();
extern void exc29();
extern void exc30();
extern void exc31();

extern void irq32();
extern void irq33();
extern void irq34();
extern void irq35();
extern void irq36();
extern void irq37();
extern void irq38();
extern void irq39();
extern void irq40();
extern void irq41();
extern void irq42();
extern void irq43();
extern void irq44();
extern void irq45();
extern void irq46();
extern void irq47();

#define ISR_MAX_INTERRUPT_LISTENERS 48

struct isr_cpu_state {
    uint32_t ds;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t interrupt_code;
    uint32_t error_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t useresp;
    uint32_t ss;
} __attribute__ ((packed));

typedef struct isr_cpu_state isr_cpu_state_t;

typedef enum {
    // CPU generated exceptions

    DIVISION_BY_ZERO_EXCEPTION = 0,
    DEBUG_EXCEPTION = 1,
    NON_MASKABLE_INTERRUPT_EXCEPTION = 2,
    BREAKPOINT_EXCEPTION = 3,
    OVERFLOW_EXCEPTION = 4,
    BOUND_RANGE_EXCEEDED_EXCEPTION = 5,
    INVALID_OPCODE_EXCEPTION = 6,
    DEVICE_NOT_AVAILABLE_EXCEPTION = 7,
    DOUBLE_FAULT_EXCEPTION = 8,
    COPROCESSOR_SEGMENT_OVERRUN_EXCEPTION = 9,
    INVALID_TSS_EXCEPTION = 10,
    SEGMENT_NOT_PRESENT_EXCEPTION = 11,
    STACK_SEGMENT_FAULT_EXCEPTION = 12,
    GENERAL_PROTECTION_EXCEPTION = 13,
    PAGE_FAULT_EXCEPTION = 14,
    RESERVED_EXCEPTION = 15,
    FLOATING_POINT_EXCEPTION = 16,
    ALIGNMENT_CHECK_EXCEPTION = 17,
    MACHINE_CHECK_EXCEPTION = 18,
    SIMD_FLOATING_POINT_EXCEPTION = 19,

    // Interrupt Request (IRQ)/Hardware interrupts

    PROGRAMMABLE_INTERRUPT_TIMER_INTERRUPT = 32,
    KEYBOARD_INTERRUPT = 33,
    CASCADE_INTERRUPT = 34,
    COM2_INTERRUPT = 35,
    COM1_INTERRUPT = 36,
    LPT2_INTERRUPT = 37,
    FLOPPY_DISK_INTERRUPT = 38,
    LPT1_INTERRUPT = 39,
    CMOS_REAL_TIME_CLOCK_INTERRUPT = 40,
    PERIPHERAL1_INTERRUPT = 41,
    PERIPHERAL2_INTERRUPT = 42,
    PERIPHERAL3_INTERRUPT = 43,
    PS2_INTERRUPT = 44,
    COPROCESSOR_INTERRUPT = 45,
    PRIMARY_ATA_HARD_DISK_INTERRUPT = 46,
    SECONDARY_ATA_HARD_DISK_INTERRUPT = 47
} isr_interrupt_t;

typedef void (*isr_interrupt_listener_t)(isr_cpu_state_t *state);

/**
 * Registers a listener to handle a specific interrupt.
 * 
 * @param selector The interrupt to install the listener for.
 * @param listener The listener to install.
 * @return 0 if the listener was successfully installed, -1 otherwise.
 */
uint32_t isr_register_listener(isr_interrupt_t selector, isr_interrupt_listener_t listener);

/**
 * Unregisters a listener from handling a specific interrupt.
 *
 * @param selector The interrupt to uninstall the listener from.
 * @return 0 if the listener was successfully uninstalled, -1 otherwise.
 */
uint32_t isr_unregister_listener(isr_interrupt_t selector);

/**
 * Enables maskable interrupts.
 */
void isr_sti();

/**
 * Disables maskable interrupts.
 */
void isr_cli();

#endif // _KERNEL_SYS_ISR_H
