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
	if(0 == listener && 32 > state->interrupt_code && 0x80 != state->interrupt_code) {
		// Check if exception did occurred in user space by checking the CPL
		if(state->cs & 0x3 == 3) {
			process_t* current_process = process_get_current();

			if(current_process) {
				current_process->exception_code = state->interrupt_code;
				process_terminate(current_process);
			}
		}

		KPANIC(KPANIC_CPU_EXCEPTION_TYPE(state->interrupt_code), isr_exception_messages[state->interrupt_code], state);
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
