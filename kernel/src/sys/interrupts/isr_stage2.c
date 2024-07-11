#include <sys/isr.h>
#include <io/ports.h>
#include <drivers/pic/8259.h>
#include <sys/kpanic.h>

isr_interrupt_listener_t listeners[ISR_MAX_INTERRUPT_LISTENERS];

const char *exception_messages[] = {
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
    isr_interrupt_listener_t listener = listeners[state->interrupt_code];

    // Call the listener
    if(0 != listener) {
        listener(state);
    }

	// Send EOI to PIC for hardware interrupts
	if(32 <= state->interrupt_code) {
		pic_8259_send_eoi(state->interrupt_code - 32);
	}

    // In case of an unhandled exception
    if(0 == listener && 32 > state->interrupt_code) {
		KPANIC(KPANIC_CPU_EXCEPTION_TYPE(state->interrupt_code), exception_messages[state->interrupt_code], state);
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
