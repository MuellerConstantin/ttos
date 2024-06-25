#include <sys/isr.h>
#include <io/ports.h>
#include <drivers/pic/8259.h>

isr_interrupt_listener_t listeners[ISR_MAX_INTERRUPT_LISTENERS];

void isr_stage2(isr_cpu_state_t *state) {
	// Call registered listener for actual handling implementation
    if(ISR_MAX_INTERRUPT_LISTENERS > state->interrupt_code && 0x00 != listeners[state->interrupt_code]) {
		isr_interrupt_listener_t listener = listeners[state->interrupt_code];
		listener(state);
	}

    // Send EOI to PIC in case of IRQ
    if(32 <= state->interrupt_code) {
        pic_8259_send_eoi(state->interrupt_code - 32);
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
