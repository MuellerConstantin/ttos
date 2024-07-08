#ifndef _KERNEL_DRIVERS_PS2_8042_H
#define _KERNEL_DRIVERS_PS2_8042_H

#include <stdint.h>
#include <stdbool.h>

#define PS2_COMMAND_REGISTER 0x64
#define PS2_STATUS_REGISTER 0x64
#define PS2_DATA_REGISTER 0x60

/**
 * Initializes the first PS/2 port by convention the keyboard port.
 * 
 * @param enable_translation Whether to enable translation for the first PS/2 port.
 */
void ps2_8042_init_first_port(bool enable_translation);

/**
 * Enables the first PS/2 port, by convention the keyboard port.
 */
void ps2_8042_enable_first_port();

/**
 * Disables the first PS/2 port, by convention the keyboard port.
 */
void ps2_8042_disable_first_port(void);

/**
 * Initializes the second PS/2 port by convention the mouse port.
 */
void ps2_8042_init_second_port();

/**
 * Enables the second PS/2 port, by convention the mouse port.
 */
void ps2_8042_enable_second_port();

/**
 * Disables the second PS/2 port, by convention the mouse port.
 */
void ps2_8042_disable_second_port();

#endif // _KERNEL_DRIVERS_PS2_8042_H