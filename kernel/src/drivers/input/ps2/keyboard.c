#include <drivers/input/ps2/keyboard.h>
#include <io/ports.h>
#include <sys/isr.h>

static void ps2_keyboard_interrupt_handler(isr_cpu_state_t *state);

static const uint32_t PS2_KEYBOARD_SET1_STANDARD_KEYCODES[] = {
    /* 00 */ 0x00, KEYBOARD_KEYCODE_ESCAPE, KEYBOARD_KEYCODE_1, KEYBOARD_KEYCODE_2, KEYBOARD_KEYCODE_3, KEYBOARD_KEYCODE_4, KEYBOARD_KEYCODE_5, KEYBOARD_KEYCODE_6,
    /* 08 */ KEYBOARD_KEYCODE_7, KEYBOARD_KEYCODE_8, KEYBOARD_KEYCODE_9, KEYBOARD_KEYCODE_0, KEYBOARD_KEYCODE_MINUS, KEYBOARD_KEYCODE_EQUAL, KEYBOARD_KEYCODE_BACKSPACE, KEYBOARD_KEYCODE_TAB,
    /* 10 */ KEYBOARD_KEYCODE_Q, KEYBOARD_KEYCODE_W, KEYBOARD_KEYCODE_E, KEYBOARD_KEYCODE_R, KEYBOARD_KEYCODE_T, KEYBOARD_KEYCODE_Y, KEYBOARD_KEYCODE_U, KEYBOARD_KEYCODE_I,
    /* 18 */ KEYBOARD_KEYCODE_O, KEYBOARD_KEYCODE_P, KEYBOARD_KEYCODE_LEFT_BRACKET, KEYBOARD_KEYCODE_RIGHT_BRACKET, KEYBOARD_KEYCODE_ENTER, KEYBOARD_KEYCODE_LEFT_CONTROL, KEYBOARD_KEYCODE_A, KEYBOARD_KEYCODE_S,
    /* 20 */ KEYBOARD_KEYCODE_D, KEYBOARD_KEYCODE_F, KEYBOARD_KEYCODE_G, KEYBOARD_KEYCODE_H, KEYBOARD_KEYCODE_J, KEYBOARD_KEYCODE_K, KEYBOARD_KEYCODE_L, KEYBOARD_KEYCODE_SEMICOLON,
    /* 28 */ KEYBOARD_KEYCODE_APOSTROPHE, KEYBOARD_KEYCODE_GRAVE, KEYBOARD_KEYCODE_LEFT_SHIFT, KEYBOARD_KEYCODE_BACKSLASH, KEYBOARD_KEYCODE_Z, KEYBOARD_KEYCODE_X, KEYBOARD_KEYCODE_C, KEYBOARD_KEYCODE_V,
    /* 30 */ KEYBOARD_KEYCODE_B, KEYBOARD_KEYCODE_N, KEYBOARD_KEYCODE_M, KEYBOARD_KEYCODE_COMMA, KEYBOARD_KEYCODE_PERIOD, KEYBOARD_KEYCODE_SLASH, KEYBOARD_KEYCODE_RIGHT_SHIFT, KEYBOARD_KEYCODE_KEYPAD_ASTERISK,
    /* 38 */ KEYBOARD_KEYCODE_LEFT_ALT, KEYBOARD_KEYCODE_SPACE, KEYBOARD_KEYCODE_CAPS_LOCK, KEYBOARD_KEYCODE_F1, KEYBOARD_KEYCODE_F2, KEYBOARD_KEYCODE_F3, KEYBOARD_KEYCODE_F4, KEYBOARD_KEYCODE_F5,
    /* 40 */ KEYBOARD_KEYCODE_F6, KEYBOARD_KEYCODE_F7, KEYBOARD_KEYCODE_F8, KEYBOARD_KEYCODE_F9, KEYBOARD_KEYCODE_F10, KEYBOARD_KEYCODE_KEYPAD_NUM_LOCK, KEYBOARD_KEYCODE_SCROLL_LOCK, KEYBOARD_KEYCODE_KEYPAD_7,
    /* 48 */ KEYBOARD_KEYCODE_KEYPAD_8, KEYBOARD_KEYCODE_KEYPAD_9, KEYBOARD_KEYCODE_KEYPAD_MINUS, KEYBOARD_KEYCODE_KEYPAD_4, KEYBOARD_KEYCODE_KEYPAD_5, KEYBOARD_KEYCODE_KEYPAD_6, KEYBOARD_KEYCODE_KEYPAD_PLUS, KEYBOARD_KEYCODE_KEYPAD_1,
    /* 50 */ KEYBOARD_KEYCODE_KEYPAD_2, KEYBOARD_KEYCODE_KEYPAD_3, KEYBOARD_KEYCODE_KEYPAD_0, KEYBOARD_KEYCODE_KEYPAD_PERIOD, 0x00, 0x00, 0x00, KEYBOARD_KEYCODE_F11,
    /* 58 */ KEYBOARD_KEYCODE_F12
};

static const uint32_t PS2_KEYBOARD_SET1_EXTENDED_KEYCODES[] = {
    /* 00 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 08 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 10 */ KEYBOARD_KEYCODE_MEDIA_PREVIOUS, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 18 */ 0x00, KEYBOARD_KEYCODE_MEDIA_NEXT, 0x00, 0x00, KEYBOARD_KEYCODE_KEYPAD_ENTER, KEYBOARD_KEYCODE_RIGHT_CONTROL, 0x00, 0x00,
    /* 20 */ KEYBOARD_KEYCODE_MEDIA_VOLUME_MUTE, KEYBOARD_KEYCODE_MEDIA_CALCULATOR, KEYBOARD_KEYCODE_MEDIA_PLAY, 0x00, KEYBOARD_KEYCODE_MEDIA_STOP, 0x00, 0x00, 0x00,
    /* 28 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, KEYBOARD_KEYCODE_MEDIA_VOLUME_DOWN, 0x00,
    /* 30 */ KEYBOARD_KEYCODE_MEDIA_VOLUME_UP, 0x00, KEYBOARD_KEYCODE_MEDIA_WWW_HOME, 0x00, 0x00, KEYBOARD_KEYCODE_KEYPAD_SLASH, 0x00, 0x00,
    /* 38 */ KEYBOARD_KEYCODE_RIGHT_ALT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* 40 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, KEYBOARD_KEYCODE_HOME,
    /* 48 */ KEYBOARD_KEYCODE_UP, KEYBOARD_KEYCODE_PAGE_UP, 0x00, KEYBOARD_KEYCODE_LEFT, 0x00, KEYBOARD_KEYCODE_RIGHT, 0x00, KEYBOARD_KEYCODE_END,
    /* 50 */ KEYBOARD_KEYCODE_DOWN, KEYBOARD_KEYCODE_PAGE_DOWN, KEYBOARD_KEYCODE_INSERT, KEYBOARD_KEYCODE_DELETE, 0x00, 0x00, 0x00, 0x00,
    /* 58 */ 0x00, 0x00, 0x00, KEYBOARD_KEYCODE_LEFT_GUI, KEYBOARD_KEYCODE_RIGHT_GUI, KEYBOARD_KEYCODE_APPLICATION, KEYBOARD_KEYCODE_ACPI_POWER, KEYBOARD_KEYCODE_ACPI_SLEEP,
    /* 60 */ 0x00, 0x00, 0x00, KEYBOARD_KEYCODE_ACPI_WAKE, 0x00, KEYBOARD_KEYCODE_MEDIA_WWW_SEARCH, KEYBOARD_KEYCODE_MEDIA_WWW_FAVORITES, KEYBOARD_KEYCODE_MEDIA_WWW_REFRESH,
    /* 68 */ KEYBOARD_KEYCODE_MEDIA_WWW_STOP, KEYBOARD_KEYCODE_MEDIA_WWW_FORWARD, KEYBOARD_KEYCODE_MEDIA_WWW_BACK, KEYBOARD_KEYCODE_MEDIA_MY_COMPUTER, KEYBOARD_KEYCODE_MEDIA_EMAIL, KEYBOARD_KEYCODE_MEDIA_SELECT
};

void ps2_keyboard_init(void) {
    isr_register_listener(KEYBOARD_INTERRUPT, ps2_keyboard_interrupt_handler);

    // Init and enable first PS/2 port
    ps2_8042_init_first_port(true);
    ps2_8042_enable_first_port();

    // Enable keyboard scanning
    outb(PS2_DATA_REGISTER, 0xF4);
}

static void ps2_keyboard_interrupt_handler(isr_cpu_state_t *state) {
    ps2_8042_disable_first_port();

    // Check if data is available
    if(inb(PS2_STATUS_REGISTER) & 0x01) {
        uint8_t scancode = inb(PS2_DATA_REGISTER);

        // Standard key pressed
        if(scancode <= 0x58) {
            uint32_t keycode = PS2_KEYBOARD_SET1_STANDARD_KEYCODES[scancode];

            if(keycode != 0x00) {
                keyboard_event_t event = {
                    .keycode = keycode,
                    .pressed = true
                };

                keyboard_buffer_enqueue(event);
            }
        // Standard key released
        } else if(scancode >= 0x80 && scancode <= 0xD8) {
            uint32_t keycode = PS2_KEYBOARD_SET1_STANDARD_KEYCODES[scancode - 0x80];

            if(keycode != 0x00) {
                keyboard_event_t event = {
                    .keycode = keycode,
                    .pressed = false
                };

                keyboard_buffer_enqueue(event);
            }
        // Pause key pressed
        } else if(scancode == 0xE1) {
            scancode = inb(PS2_DATA_REGISTER);

            if(scancode == 0x1D) {
                scancode = inb(PS2_DATA_REGISTER);

                if(scancode == 0x45) {
                    scancode = inb(PS2_DATA_REGISTER);

                    if(scancode == 0xE1) {
                        scancode = inb(PS2_DATA_REGISTER);

                        if(scancode == 0x9D) {
                            scancode = inb(PS2_DATA_REGISTER);

                            if(scancode == 0xC5) {
                                keyboard_event_t event = {
                                    .keycode = KEYBOARD_KEYCODE_PAUSE,
                                    .pressed = true
                                };

                                keyboard_buffer_enqueue(event);
                            }
                        }
                    }
                }
            }
        // Extended key pressed/released
        } else if(scancode == 0xE0) {
            if(inb(PS2_STATUS_REGISTER) & 0x01) {
                scancode = inb(PS2_DATA_REGISTER);

                // Print screen key pressed
                if(scancode == 0x2A) {
                    scancode = inb(PS2_DATA_REGISTER);

                    if(scancode == 0xE0) {
                        scancode = inb(PS2_DATA_REGISTER);

                        if(scancode == 0x37) {
                            keyboard_event_t event = {
                                .keycode = KEYBOARD_KEYCODE_PRINT_SCREEN,
                                .pressed = true
                            };

                            keyboard_buffer_enqueue(event);
                        }
                    }
                // Print screen key released
                }
                else if(scancode == 0xB7) {
                    scancode = inb(PS2_DATA_REGISTER);

                    if(scancode == 0xE0) {
                        scancode = inb(PS2_DATA_REGISTER);

                        if(scancode == 0xAA) {
                            keyboard_event_t event = {
                                .keycode = KEYBOARD_KEYCODE_PRINT_SCREEN,
                                .pressed = false
                            };

                            keyboard_buffer_enqueue(event);
                        }
                    }
                // Extended key pressed
                } else if(scancode <= 0x6D) {
                    uint8_t keycode = PS2_KEYBOARD_SET1_EXTENDED_KEYCODES[scancode];

                    if(keycode != 0x00) {
                        keyboard_event_t event = {
                            .keycode = keycode,
                            .pressed = true
                        };

                        keyboard_buffer_enqueue(event);
                    }
                // Extended key released
                } else if(scancode >= 0x80 && scancode <= 0xED) {
                    uint8_t keycode = PS2_KEYBOARD_SET1_EXTENDED_KEYCODES[scancode - 0x80];

                    if(keycode != 0x00) {
                        keyboard_event_t event = {
                            .keycode = keycode,
                            .pressed = false
                        };

                        keyboard_buffer_enqueue(event);
                    }
                }
            }
        }
    }

    ps2_8042_enable_first_port();
}