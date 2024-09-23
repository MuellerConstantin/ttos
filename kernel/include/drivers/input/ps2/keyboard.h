#ifndef _KERNEL_DRIVERS_PS2_KEYBOARD_H
#define _KERNEL_DRIVERS_PS2_KEYBOARD_H

#include <drivers/input/ps2/8042.h>
#include <device/keyboard.h>

#define KEYBOARD_BUFFER_SIZE 256

/**
 * Initializes the PS/2 keyboard.
 * 
 * @return 0 if the PS/2 keyboard was successfully initialized, otherwise an error code.
 */
int32_t ps2_keyboard_init();

/**
 * Retrieves the next keyboard event from the keyboard buffer.
 * 
 * @param event The keyboard event to store the retrieved event.
 */
void ps2_keyboard_dequeue(keyboard_event_t* event);

/**
 * Checks if there is a keyboard event available in the keyboard buffer.
 * 
 * @return Whether there is a keyboard event available in the keyboard buffer.
 */
bool ps2_keyboard_available();

#endif // _KERNEL_DRIVERS_PS2_KEYBOARD_H
