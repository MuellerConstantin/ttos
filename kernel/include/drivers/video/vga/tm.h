#ifndef _KERNEL_DRIVERS_VIDEO_VGA_TM_H
#define _KERNEL_DRIVERS_VIDEO_VGA_TM_H

#include <drivers/video/vga/vga.h>
#include <string.h>

#define VGA_TM_ENTRY(ch, fgcolor, bgcolor) (ch | ((((bgcolor & 0x0F) << 4) | (fgcolor & 0x0F)) << 8))

#define VGA_TM_BLACK			0x00
#define VGA_TM_BLUE             0x01
#define VGA_TM_GREEN			0x02
#define VGA_TM_CYAN             0x03
#define VGA_TM_RED				0x04
#define VGA_TM_MAGENTA			0x05
#define VGA_TM_BROWN			0x06
#define VGA_TM_LIGHT_GREY		0x07
#define VGA_TM_DARK_GREY		0x08
#define VGA_TM_LIGHT_BLUE		0x09
#define VGA_TM_DARK_GREEN		0x0A
#define VGA_TM_LIGHT_CYAN		0x0B
#define VGA_TM_LIGHT_RED		0x0C
#define VGA_TM_LIGHT_MAGENTA	0x0D
#define VGA_TM_LIGHT_BROWN		0x0E
#define VGA_TM_WHITE			0x0F

#define VGA_TM_CURSOR_MIN_SCANLINE 0x00
#define VGA_TM_CURSOR_MAX_SCANLINE 0x0F

typedef struct vga_tm_screen vga_tm_screen_t;

struct vga_tm_screen {
    size_t rows;
    size_t columns;
    size_t cursor_x;
    size_t cursor_y;
    uint8_t fgcolor;
    uint8_t bgcolor;
};

/**
 * Initialize the VGA text mode driver.
 * 
 * @return 0 if successful, -1 if the driver failed to initialize.
 */
int32_t vga_tm_init();

/**
 * Enable the VGA text mode cursor.
 * 
 * @param start The start scanline.
 * @param end The end scanline.
 */
void vga_tm_enable_cursor(uint8_t start, uint8_t end);

/**
 * Disable the VGA text mode cursor.
 */
void vga_tm_disable_cursor();

/**
 * Move the VGA text mode cursor.
 * 
 * @param cell The cell index to move to.
 * @return 0 if successful, -1 if the cell index is out of bounds.
 */
int32_t vga_tm_move_cursor(size_t cell);

/**
 * Clear the VGA text mode buffer.
 */
void vga_tm_clear();

/**
 * Fill the VGA text mode buffer with a background color.
 * 
 * @param bgcolor The background color.
 */
void vga_tm_fill(uint8_t bgcolor);

/**
 * Write a character to the VGA text mode buffer.
 * 
 * @param cell The cell index to write to.
 * @param ch The character to write.
 * @param fgcolor The foreground color.
 * @param bgcolor The background color.
 * @return 0 if successful, -1 if the cell index is out of bounds.
 */
int32_t vga_tm_write(size_t cell, uint8_t ch, uint8_t fgcolor, uint8_t bgcolor);

/**
 * Write a string to the VGA text mode buffer.
 * 
 * @param offset The offset to write to.
 * @param str The string to write.
 * @param fgcolor The foreground color.
 * @param bgcolor The background color.
 * @return 0 if successful, -1 if the cell index is out of bounds.
 */
int32_t vga_tm_strwrite(size_t offset, const char* str, uint8_t fgcolor, uint8_t bgcolor);

/**
 * Write a character to the VGA text mode buffer at the current cursor position.
 * 
 * @param ch The character to write.
 */
void vga_tm_putchar(char ch);

/**
 * Write a string to the VGA text mode buffer at the current cursor position.
 * 
 * @param str The string to write.
 */
void vga_tm_putstr(const char *str);

/**
 * Scroll the VGA text mode buffer up by one line.
 */
void vga_tm_scroll();

/**
 * Changes the foreground and background colors.
 * 
 * @param fgcolor The foreground color.
 * @param bgcolor The background color.
 */
void vga_tm_set_color(uint8_t fgcolor, uint8_t bgcolor);

#endif // _KERNEL_DRIVERS_VIDEO_VGA_TM_H
