#include <arch/i386/isr.h>
#include <system/ports.h>
#include <arch/i386/pic/8259.h>
#include <system/kpanic.h>
#include <system/process.h>

isr_interrupt_listener_t listeners[ISR_MAX_INTERRUPT_LISTENERS];

const char *isr_exception_messages[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Detected Overflow",
	"Out Of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

void isr_stage2(isr_cpu_state_t *state) {
	// Send EOI to the PIC for hardware interrupts before dispatching the
	// listener. A listener may perform a non-local exit (e.g. Ctrl+C tearing
	// down the foreground process via process_exit), which would otherwise
	// skip the EOI and leave the PIC unable to deliver further interrupts.
	if(32 <= state->interrupt_code) {
		pic_8259_send_eoi(state->interrupt_code - 32);
	}

    isr_interrupt_listener_t listener = listeners[state->interrupt_code];

    // Call the listener
    if(0 != listener) {
        listener(state);
    }

	// The interrupt came from user space if the saved cs carries RPL 3.
	bool from_user = (state->cs & 0x3) == 3;

    // In case of an unhandled exception
	if(0 == listener && 32 > state->interrupt_code && 0x80 != state->interrupt_code) {
		if(from_user && process_get_current()) {
			process_exit(0, state->interrupt_code);
		}

		KPANIC(KPANIC_CPU_EXCEPTION_TYPE(state->interrupt_code), isr_exception_messages[state->interrupt_code], state);
	}

	/*
	 * Only a return to user space may end or switch processes. A frame that
	 * returns into the kernel (the idle loop, a nested interrupt) is left
	 * alone, so kernel code is never preempted.
	 */
	if(from_user) {
		process_deliver_kill();
		process_preempt();
	}
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
