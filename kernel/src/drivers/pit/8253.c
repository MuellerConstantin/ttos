#include <drivers/pit/8253.h>
#include <sys/isr.h>

static volatile size_t pit_8253_ticks;

static void pit_8253_interrupt_handler(isr_cpu_state_t *state);

int32_t pit_8253_init(uint8_t counter, uint32_t frequency) {
    uint32_t divisor = PIT_8253_OSCILLATOR_FREQUENCY / frequency;

    uint16_t data_register = 0;

    switch (counter) {
        case PIT_8253_COUNTER_0:
            data_register = PIT_8253_COUNTER_0_DATA_REGISTER;
            break;
        case PIT_8253_COUNTER_1:
            data_register = PIT_8253_COUNTER_1_DATA_REGISTER;
            break;
        case PIT_8253_COUNTER_2:
            data_register = PIT_8253_COUNTER_2_DATA_REGISTER;
            break;
        default:
            return -1;
    }

    outb(PIT_8253_COMMAND_REGISTER, 0x36);
    outb(data_register, divisor & 0xFF);
    outb(data_register, divisor >> 8);

    pit_8253_ticks = 0;

    isr_register_listener(PROGRAMMABLE_INTERRUPT_TIMER_INTERRUPT, pit_8253_interrupt_handler);

    return 0;
}

static void pit_8253_interrupt_handler(isr_cpu_state_t *state) {
    ++pit_8253_ticks;
}

void pit_8253_wait(size_t ticks) {
    size_t start = pit_8253_ticks;
    while (pit_8253_ticks - start < ticks);
}
