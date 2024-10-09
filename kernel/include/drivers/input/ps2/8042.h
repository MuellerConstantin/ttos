/**
 * @file 8042.h
 * @brief Definitions for the 8042 PS/2 controller.
 * 
 * The 8042 PS/2 controller is a chip that interfaces the PS/2 keyboard and mouse with the system.
 * In modern systems, the 8042 controller is not present and the PS/2 controller is integrated into
 * the Advanced Integrated Peripheral (AIP) chipset.
 */

#ifndef _KERNEL_DRIVERS_PS2_8042_H
#define _KERNEL_DRIVERS_PS2_8042_H

#include <stdint.h>
#include <stdbool.h>

#define PS2_COMMAND_REGISTER 0x64
#define PS2_STATUS_REGISTER 0x64
#define PS2_DATA_REGISTER 0x60

/**
 * Probes the first PS/2 port.
 * 
 * @return Whether the first PS/2 port is present.
 */
bool ps2_8042_first_port_probe();

/**
 * Enables the first PS/2 port, by convention the keyboard port.
 */
void ps2_8042_enable_first_port();

/**
 * Disables the first PS/2 port, by convention the keyboard port.
 */
void ps2_8042_disable_first_port(void);

/**
 * Initializes the first PS/2 port by convention the keyboard port.
 * 
 * @param enable_translation Whether to enable translation for the first PS/2 port.
 */
void ps2_8042_init_first_port(bool enable_translation);

/**
 * Probes the second PS/2 port.
 * 
 * @return Whether the second PS/2 port is present.
 */
bool ps2_8042_second_port_probe();

/**
 * Enables the second PS/2 port, by convention the mouse port.
 */
void ps2_8042_enable_second_port();

/**
 * Disables the second PS/2 port, by convention the mouse port.
 */
void ps2_8042_disable_second_port();

/**
 * Initializes the second PS/2 port by convention the mouse port.
 */
void ps2_8042_init_second_port();

#endif // _KERNEL_DRIVERS_PS2_8042_H
