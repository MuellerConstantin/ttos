#ifndef _KERNEL_DRIVERS_PIT_8253_H
#define _KERNEL_DRIVERS_PIT_8253_H

#include <stdint.h>
#include <stddef.h>
#include <system/ports.h>
#include <device/device.h>

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

/**
 * Registers the timer as a device, or returns it when it already is one.
 * Bringing the counters up is safe at any time and says nothing about the
 * device model, so announcing the chip is a separate step that has to wait
 * until the model is ready.
 *
 * @return The timer device.
 */
device_t* pit_8253_claim_device(void);

#endif // _KERNEL_DRIVERS_PIT_8253_H
