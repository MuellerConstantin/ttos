#ifndef _KERNEL_DRIVERS_PS2_KEYBOARD_H
#define _KERNEL_DRIVERS_PS2_KEYBOARD_H

#include <drivers/input/keyboard.h>
#include <drivers/input/ps2/8042.h>

/**
 * Initializes the PS/2 keyboard.
 * 
 * @return 0 if the PS/2 keyboard was successfully initialized, otherwise an error code.
 */
int32_t ps2_keyboard_init();

#endif // _KERNEL_DRIVERS_PS2_KEYBOARD_H
