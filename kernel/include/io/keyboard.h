#ifndef _KERNEL_IO_KEYBOARD_H
#define _KERNEL_IO_KEYBOARD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define KEYBOARD_KEYCODE(t, v)      (((t) << 8) | (v))
#define KEYBOARD_KEYCODE_TYPE(x)    ((x) >> 8)
#define KEYBOARD_KEYCODE_VALUE(x)   ((x) & 0xFF)

#define KEYBOARD_KEYCODE_TYPE_LATIN     1
#define KEYBOARD_KEYCODE_TYPE_FUNCTION  2
#define KEYBOARD_KEYCODE_TYPE_CURSOR    3
#define KEYBOARD_KEYCODE_TYPE_KEYPAD    4
#define KEYBOARD_KEYCODE_TYPE_CONTROL   5
#define KEYBOARD_KEYCODE_TYPE_MEDIA     6
#define KEYBOARD_KEYCODE_TYPE_ACPI      7

#define KEYBOARD_KEYCODE_A              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 0)
#define KEYBOARD_KEYCODE_B              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 1)
#define KEYBOARD_KEYCODE_C              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 2)
#define KEYBOARD_KEYCODE_D              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 3)
#define KEYBOARD_KEYCODE_E              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 4)
#define KEYBOARD_KEYCODE_F              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 5)
#define KEYBOARD_KEYCODE_G              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 6)
#define KEYBOARD_KEYCODE_H              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 7)
#define KEYBOARD_KEYCODE_I              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 8)
#define KEYBOARD_KEYCODE_J              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 9)
#define KEYBOARD_KEYCODE_K              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 10)
#define KEYBOARD_KEYCODE_L              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 11)
#define KEYBOARD_KEYCODE_M              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 12)
#define KEYBOARD_KEYCODE_N              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 13)
#define KEYBOARD_KEYCODE_O              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 14)
#define KEYBOARD_KEYCODE_P              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 15)
#define KEYBOARD_KEYCODE_Q              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 16)
#define KEYBOARD_KEYCODE_R              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 17)
#define KEYBOARD_KEYCODE_S              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 18)
#define KEYBOARD_KEYCODE_T              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 19)
#define KEYBOARD_KEYCODE_U              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 20)
#define KEYBOARD_KEYCODE_V              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 21)
#define KEYBOARD_KEYCODE_W              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 22)
#define KEYBOARD_KEYCODE_X              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 23)
#define KEYBOARD_KEYCODE_Y              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 24)
#define KEYBOARD_KEYCODE_Z              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 25)
#define KEYBOARD_KEYCODE_0              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 26)
#define KEYBOARD_KEYCODE_1              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 27)
#define KEYBOARD_KEYCODE_2              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 28)
#define KEYBOARD_KEYCODE_3              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 29)
#define KEYBOARD_KEYCODE_4              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 30)
#define KEYBOARD_KEYCODE_5              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 31)
#define KEYBOARD_KEYCODE_6              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 32)
#define KEYBOARD_KEYCODE_7              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 33)
#define KEYBOARD_KEYCODE_8              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 34)
#define KEYBOARD_KEYCODE_9              KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 35)
#define KEYBOARD_KEYCODE_EQUAL          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 36)
#define KEYBOARD_KEYCODE_MINUS          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 37)
#define KEYBOARD_KEYCODE_LEFT_BRACKET   KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 38)
#define KEYBOARD_KEYCODE_RIGHT_BRACKET  KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 39)
#define KEYBOARD_KEYCODE_SEMICOLON      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 40)
#define KEYBOARD_KEYCODE_APOSTROPHE     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 41)
#define KEYBOARD_KEYCODE_GRAVE          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 42)
#define KEYBOARD_KEYCODE_COMMA          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 43)
#define KEYBOARD_KEYCODE_PERIOD         KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 44)
#define KEYBOARD_KEYCODE_SLASH          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 45)
#define KEYBOARD_KEYCODE_BACKSLASH      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 46)
#define KEYBOARD_KEYCODE_SPACE          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_LATIN, 47)

#define KEYBOARD_KEYCODE_F1             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 0)
#define KEYBOARD_KEYCODE_F2             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 1)
#define KEYBOARD_KEYCODE_F3             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 2)
#define KEYBOARD_KEYCODE_F4             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 3)
#define KEYBOARD_KEYCODE_F5             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 4)
#define KEYBOARD_KEYCODE_F6             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 5)
#define KEYBOARD_KEYCODE_F7             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 6)
#define KEYBOARD_KEYCODE_F8             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 7)
#define KEYBOARD_KEYCODE_F9             KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 8)
#define KEYBOARD_KEYCODE_F10            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 9)
#define KEYBOARD_KEYCODE_F11            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 10)
#define KEYBOARD_KEYCODE_F12            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 11)
#define KEYBOARD_KEYCODE_PRINT_SCREEN   KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 12)
#define KEYBOARD_KEYCODE_SCROLL_LOCK    KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 13)
#define KEYBOARD_KEYCODE_PAUSE          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 14)
#define KEYBOARD_KEYCODE_INSERT         KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 15)
#define KEYBOARD_KEYCODE_DELETE         KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 16)
#define KEYBOARD_KEYCODE_HOME           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 17)
#define KEYBOARD_KEYCODE_END            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 18)
#define KEYBOARD_KEYCODE_PAGE_UP        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 19)
#define KEYBOARD_KEYCODE_PAGE_DOWN      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_FUNCTION, 20)

#define KEYBOARD_KEYCODE_LEFT          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CURSOR, 0)
#define KEYBOARD_KEYCODE_RIGHT         KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CURSOR, 1)
#define KEYBOARD_KEYCODE_UP            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CURSOR, 2)
#define KEYBOARD_KEYCODE_DOWN          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CURSOR, 3)

#define KEYBOARD_KEYCODE_KEYPAD_0           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 0)
#define KEYBOARD_KEYCODE_KEYPAD_1           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 1)
#define KEYBOARD_KEYCODE_KEYPAD_2           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 2)
#define KEYBOARD_KEYCODE_KEYPAD_3           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 3)
#define KEYBOARD_KEYCODE_KEYPAD_4           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 4)
#define KEYBOARD_KEYCODE_KEYPAD_5           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 5)
#define KEYBOARD_KEYCODE_KEYPAD_6           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 6)
#define KEYBOARD_KEYCODE_KEYPAD_7           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 7)
#define KEYBOARD_KEYCODE_KEYPAD_8           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 8)
#define KEYBOARD_KEYCODE_KEYPAD_9           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 9)
#define KEYBOARD_KEYCODE_KEYPAD_PERIOD      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 10)
#define KEYBOARD_KEYCODE_KEYPAD_SLASH       KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 11)
#define KEYBOARD_KEYCODE_KEYPAD_PLUS        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 12)
#define KEYBOARD_KEYCODE_KEYPAD_MINUS       KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 13)
#define KEYBOARD_KEYCODE_KEYPAD_ENTER       KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 14)
#define KEYBOARD_KEYCODE_KEYPAD_ASTERISK    KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 15)
#define KEYBOARD_KEYCODE_KEYPAD_NUM_LOCK    KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_KEYPAD, 16)

#define KEYBOARD_KEYCODE_LEFT_CONTROL  KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 0)
#define KEYBOARD_KEYCODE_LEFT_SHIFT    KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 1)
#define KEYBOARD_KEYCODE_LEFT_ALT      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 2)
#define KEYBOARD_KEYCODE_CAPS_LOCK     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 3)
#define KEYBOARD_KEYCODE_TAB           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 4)
#define KEYBOARD_KEYCODE_BACKSPACE     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 5)
#define KEYBOARD_KEYCODE_ENTER         KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 6)
#define KEYBOARD_KEYCODE_RIGHT_SHIFT   KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 7)
#define KEYBOARD_KEYCODE_RIGHT_CONTROL KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 8)
#define KEYBOARD_KEYCODE_RIGHT_ALT     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 9)
#define KEYBOARD_KEYCODE_ESCAPE        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 10)
#define KEYBOARD_KEYCODE_LEFT_GUI      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 11)
#define KEYBOARD_KEYCODE_RIGHT_GUI     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 12)
#define KEYBOARD_KEYCODE_APPLICATION    KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_CONTROL, 13)

#define KEYBOARD_KEYCODE_MEDIA_VOLUME_UP       KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 0)
#define KEYBOARD_KEYCODE_MEDIA_VOLUME_DOWN     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 1)
#define KEYBOARD_KEYCODE_MEDIA_VOLUME_MUTE     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 2)
#define KEYBOARD_KEYCODE_MEDIA_PLAY            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 3)
#define KEYBOARD_KEYCODE_MEDIA_STOP            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 4)
#define KEYBOARD_KEYCODE_MEDIA_PREVIOUS        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 5)
#define KEYBOARD_KEYCODE_MEDIA_NEXT            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 6)
#define KEYBOARD_KEYCODE_MEDIA_PAUSE           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 7)
#define KEYBOARD_KEYCODE_MEDIA_SELECT          KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 8)
#define KEYBOARD_KEYCODE_MEDIA_CALCULATOR      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 9)
#define KEYBOARD_KEYCODE_MEDIA_EMAIL           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 10)
#define KEYBOARD_KEYCODE_MEDIA_WWW_HOME        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 11)
#define KEYBOARD_KEYCODE_MEDIA_WWW_SEARCH      KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 12)
#define KEYBOARD_KEYCODE_MEDIA_WWW_FAVORITES   KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 13)
#define KEYBOARD_KEYCODE_MEDIA_WWW_REFRESH     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 14)
#define KEYBOARD_KEYCODE_MEDIA_WWW_STOP        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 15)
#define KEYBOARD_KEYCODE_MEDIA_WWW_FORWARD     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 16)
#define KEYBOARD_KEYCODE_MEDIA_WWW_BACK        KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 17)
#define KEYBOARD_KEYCODE_MEDIA_MY_COMPUTER     KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_MEDIA, 18)

#define KEYBOARD_KEYCODE_ACPI_POWER           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_ACPI, 0)
#define KEYBOARD_KEYCODE_ACPI_SLEEP           KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_ACPI, 1)
#define KEYBOARD_KEYCODE_ACPI_WAKE            KEYBOARD_KEYCODE(KEYBOARD_KEYCODE_TYPE_ACPI, 2)

#define KEYBOARD_BUFFER_SIZE 256

struct keyboard_event {
    uint32_t keycode;
    bool pressed;
};

typedef struct keyboard_event keyboard_event_t;

/**
 * Pushes a keyboard event into the keyboard buffer.
 * 
 * @param event The keyboard event to push.
 */
void keyboard_buffer_enqueue(keyboard_event_t event);

/**
 * Retrieves the next keyboard event from the keyboard buffer.
 * 
 * @return The keyboard event that was retrieved.
 */
keyboard_event_t keyboard_buffer_dequeue();

/**
 * Checks if there is a keyboard event available in the keyboard buffer.
 * 
 * @return Whether there is a keyboard event available in the keyboard buffer.
 */
bool keyboard_buffer_available();

#endif // _KERNEL_IO_KEYBOARD_H
