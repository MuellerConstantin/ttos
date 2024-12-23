#ifndef _KERNEL_IO_TTY_H
#define _KERNEL_IO_TTY_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <device/device.h>
#include <io/stream.h>
#include <util/circular_buffer.h>

#define TTY_BUFFER_SIZE 1024

#define TTY_BLACK			0x00
#define TTY_BLUE            0x01
#define TTY_GREEN			0x02
#define TTY_CYAN            0x03
#define TTY_RED				0x04
#define TTY_MAGENTA			0x05
#define TTY_BROWN			0x06
#define TTY_LIGHT_GREY		0x07
#define TTY_DARK_GREY		0x08
#define TTY_LIGHT_BLUE		0x09
#define TTY_DARK_GREEN		0x0A
#define TTY_LIGHT_CYAN		0x0B
#define TTY_LIGHT_RED		0x0C
#define TTY_LIGHT_MAGENTA	0x0D
#define TTY_LIGHT_BROWN		0x0E
#define TTY_WHITE			0x0F

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
    size_t rows;
    size_t columns;
    size_t cursor_x;
    size_t cursor_y;
    uint8_t fgcolor;
    uint8_t bgcolor;
    circular_buffer_t* input;
    circular_buffer_t* output;
    video_device_t* video;
    keyboard_device_t* keyboard;
    tty_keyboard_layout_t* layout;
};

extern tty_keyboard_layout_t tty_keyboard_layout_de_DE;

/**
 * Sets the TTY to the standard terminal.
 * 
 * @param tty The TTY.
 */
void tty_set_stdterm(tty_t* tty);

/**
 * Gets the TTY of the standard terminal.
 * 
 * @return The TTY.
 */
const tty_t* tty_get_stdterm();

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
 * Gets the output stream of the TTY.
 * 
 * @param tty The TTY.
 * @return The output stream.
 */
stream_t* tty_get_out_stream(tty_t* tty);

/**
 * Gets the input stream of the TTY.
 * 
 * @param tty The TTY.
 * @return The input stream.
 */
stream_t* tty_get_in_stream(tty_t* tty);

/**
 * Gets the error stream of the TTY.
 * 
 * @param tty The TTY.
 * @return The error stream.
 */
stream_t* tty_get_err_stream(tty_t* tty);

/**
 * Flushes the TTY.
 * 
 * @param tty The TTY.
 */
void tty_flush(tty_t* tty);

/**
 * Clears the TTY.
 * 
 * @param tty The TTY.
 */
void tty_clear(tty_t* tty0);

/**
 * Writes a character to the TTY.
 * 
 * @param tty The TTY.
 * @param c The character to write.
 */
void tty_putchar(tty_t* tty0, char c);

/**
 * Reads a character from the TTY. In contrast to the other functions of the TTY,
 * this function is implemented in raw mode and not in canonical mode, meaning
 * it does not block and echoing is disabled.
 * 
 * @param tty The TTY.
 * @return The character read.
 */
char tty_getchar(tty_t* tty0);

/**
 * Writes a string to the TTY.
 * 
 * @param tty The TTY.
 * @param str The string to write.
 */
void tty_puts(tty_t* tty, const char* str);

/**
 * Reads a line from the TTY.
 * 
 * @param tty The TTY.
 * @return The line read.
 */
char* tty_gets(tty_t* tty);

/**
 * Changes the foreground color of the TTY.
 * 
 * @param tty The TTY.
 * @param fgcolor The foreground color.
 */
void tty_set_fgcolor(tty_t* tty0, uint8_t fgcolor);

/**
 * Changes the background color of the TTY.
 * 
 * @param tty The TTY.
 * @param bgcolor The background color.
 */
void tty_set_bgcolor(tty_t* tty0, uint8_t bgcolor);

/**
 * Disables the cursor of the TTY.
 * 
 * @param tty The TTY.
 */
void tty_disable_cursor(tty_t* tty0);

/**
 * Enables the cursor of the TTY.
 * 
 * @param tty The TTY.
 */
void tty_enable_cursor(tty_t* tty0);

#endif // _KERNEL_IO_TTY_H
