#ifndef _KERNEL_IO_TTY_H
#define _KERNEL_IO_TTY_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <device/device.h>

typedef struct tty_keymap_entry tty_keymap_entry_t;
typedef struct tty_keyboard_layout tty_keyboard_layout_t;

struct tty_keymap_entry {
    uint32_t keycode;
    char normal;
    char shifted;
};

struct tty_keyboard_layout {
    char name[32];
    tty_keymap_entry_t* keymap;
    size_t keymap_size;
};

typedef struct tty tty_t;

struct tty {
    video_device_t* video;
    keyboard_device_t* keyboard;
    tty_keyboard_layout_t* layout;
};

extern tty_keyboard_layout_t tty_keyboard_layout_de_DE;

/**
 * Creates a new TTY.
 * 
 * @param video The video device to use.
 * @param keyboard The keyboard device to use.
 * @param layout The keyboard layout to use.
 * @return The TTY.
 */
tty_t* tty_create(video_device_t* video, keyboard_device_t* keyboard, tty_keyboard_layout_t* layout);

/**
 * Writes a character to the TTY.
 * 
 * @param tty The TTY.
 * @param c The character to write.
 */
void tty_putchar(tty_t* tty0, char c);

/**
 * Reads a character from the TTY.
 * 
 * @param tty The TTY.
 * @return The character read.
 */
char tty_getchar(tty_t* tty0);

/**
 * Reads a line from a stream.
 * 
 * @param putchar The function to write a character to the stream, used for echo.
 * @param getchar The function to read a character from the stream.
 * @param echo Whether to echo the input.
 * @return The line read from the stream.
 */
char* tty_readline(tty_t* tty0, bool echo);

/**
 * Naive printf implementation that writes to a stream. This function does not
 * support all the features of the standard printf function.
 * 
 * @param putchar The function to write a character to the stream.
 * @param format The format string.
 * @param ... The arguments to format.
 * @return The number of characters written.
 */
int tty_printf(tty_t* tty0, const char *format, ...);

/**
 * Naive vprintf implementation that writes to a stream. This function does not
 * support all the features of the standard vprintf function.
 * 
 * @param putchar The function to write a character to the stream.
 * @param format The format string.
 * @param args The arguments to format.
 * @return The number of characters written.
 */
int tty_vprintf(tty_t* tty0, const char *format, va_list args);

#endif // _KERNEL_IO_TTY_H
