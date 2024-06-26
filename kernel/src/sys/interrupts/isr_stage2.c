#include <sys/isr.h>
#include <io/ports.h>
#include <drivers/pic/8259.h>
#include <sys/panic.h>

isr_interrupt_listener_t listeners[ISR_MAX_INTERRUPT_LISTENERS];

const char *exception_messages[] = {
	"DIVISION BY ZERO",
	"DEBUG",
	"NON-MASKABLE INTERUPT",
	"BREAKPOINT",
	"DETECTED OVERFLOW",
	"OUT-OF-BOUNDS",
	"INVALID OPCODE",
	"NO COPROCESSOR",
	"DOUBLE FAULT",
	"COPROCESSOR SEGMENT OVERRUN",
	"BAD TSS",
	"SEGMENT NOT PRESENT",
	"STACK FAULT",
	"GENERAL PROTECTION FAULT",
	"PAGE FAULT",
	"UNKNOWN INTERRUPT",
	"COPROCESSOR FAULT",
	"ALIGNMENT CHECK",
	"MACHINE CHECK",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED",
	"RESERVED"
};

void isr_stage2(isr_cpu_state_t *state) {
    isr_interrupt_listener_t listener = listeners[state->interrupt_code];

    // If interrupt is handled or is a hardware interrupt
    if(0 != listener || 32 <= state->interrupt_code) {
        listener(state);

        // Send EOI to PIC
        if(32 <= state->interrupt_code) {
            pic_8259_send_eoi(state->interrupt_code - 32);
        }

        return;
    }

    // In case of an unhandled exception
    kpanic(exception_messages[state->interrupt_code], state);
}

uint32_t isr_register_listener(isr_interrupt_t selector, isr_interrupt_listener_t listener) {
    if(ISR_MAX_INTERRUPT_LISTENERS <= selector) {
        return -1;
    }

    listeners[selector] = listener;
    return 0;
}

uint32_t isr_unregister_listener(isr_interrupt_t selector) {
    if(ISR_MAX_INTERRUPT_LISTENERS <= selector) {
        return -1;
    }

    listeners[selector] = 0x00;
    return 0;
}