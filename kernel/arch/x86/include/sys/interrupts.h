#ifndef _KERNEL_ARCH_X86_SYS_INTERRUPTS_H
#define _KERNEL_ARCH_X86_SYS_INTERRUPTS_H

#include <stdint.h>

#define MAX_INTERRUPT_LISTENERS 48

struct processor_state {
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

typedef struct processor_state processor_state_t;

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
} interrupt_t;

typedef void (*interrupt_listener_t)(processor_state_t *state);

/**
 * Registers a listener to handle a specific interrupt.
 * 
 * @param selector The interrupt to install the listener for.
 * @param listener The listener to install.
 * @return 0 if the listener was successfully installed, -1 otherwise.
 */
uint32_t interrupt_register_listener(interrupt_t selector, interrupt_listener_t listener);

/**
 * Unregisters a listener from handling a specific interrupt.
 *
 * @param selector The interrupt to uninstall the listener from.
 * @return 0 if the listener was successfully uninstalled, -1 otherwise.
 */
uint32_t interrupt_unregister_listener(interrupt_t selector);

#endif // _KERNEL_ARCH_X86_SYS_INTERRUPTS_H
