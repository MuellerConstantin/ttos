#include <drivers/pit/8253.h>
#include <arch/i386/isr.h>

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

    return 0;
}
