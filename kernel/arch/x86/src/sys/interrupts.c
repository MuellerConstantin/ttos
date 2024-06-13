#include <sys/interrupts.h>

interrupt_listener_t listeners[MAX_INTERRUPT_LISTENERS];

void interrupt_handler(processor_state_t *state) {
	// Call registered listener for actual handling implementation
    if(MAX_INTERRUPT_LISTENERS > state->interrupt_code && 0x00 != listeners[state->interrupt_code]) {
		interrupt_listener_t listener = listeners[state->interrupt_code];
		listener(state);
	}
}

uint32_t interrupt_register_listener(interrupt_t selector, interrupt_listener_t listener) {
    if(MAX_INTERRUPT_LISTENERS <= selector) {
        return -1;
    }

    listeners[selector] = listener;
    return 0;
}

uint32_t interrupt_unregister_listener(interrupt_t selector) {
    if(MAX_INTERRUPT_LISTENERS <= selector) {
        return -1;
    }

    listeners[selector] = 0x00;
    return 0;
}
