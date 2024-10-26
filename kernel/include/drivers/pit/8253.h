#ifndef _KERNEL_DRIVERS_PIT_8253_H
#define _KERNEL_DRIVERS_PIT_8253_H

#include <stdint.h>
#include <stddef.h>
#include <system/ports.h>

#define PIT_8253_COUNTER_0 0
#define PIT_8253_COUNTER_1 1
#define PIT_8253_COUNTER_2 2

#define PIT_8253_COUNTER_0_DATA_REGISTER 0x40
#define PIT_8253_COUNTER_1_DATA_REGISTER 0x41
#define PIT_8253_COUNTER_2_DATA_REGISTER 0x42
#define PIT_8253_COMMAND_REGISTER        0x43

#define PIT_8253_OSCILLATOR_FREQUENCY   1193180

/**
 * Initialize the 8253 PIT.
 * 
 * @param counter   Counter to use.
 * @param frequency Frequency to set.
 */
int32_t pit_8253_init(uint8_t counter, uint32_t frequency);

#endif // _KERNEL_DRIVERS_PIT_8253_H
