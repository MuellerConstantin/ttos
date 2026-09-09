#ifndef _KERNEL_DRIVERS_VIDEO_VGA_TM_H
#define _KERNEL_DRIVERS_VIDEO_VGA_TM_H

#include <drivers/video/vga/vga.h>
#include <util/string.h>

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

/*
 * The system's background shade. VGA's own blue is a saturated #0000AA, which is
 * far too loud to sit behind a whole screen of text, so the DAC entry it points
 * at is reprogrammed at init to this darker blue-grey. Attribute color 1 —
 * TTY_BLUE, and with it ESC [ 34 m and ESC [ 44 m — renders in it afterwards.
 *
 * Components are 6 bit (0-63). The values below are roughly #2A4A66.
 */
#define VGA_TM_THEME_COLOR_INDEX    0x01
#define VGA_TM_THEME_COLOR_RED      10
#define VGA_TM_THEME_COLOR_GREEN    18
#define VGA_TM_THEME_COLOR_BLUE     25

#define VGA_TM_CURSOR_MIN_SCANLINE 0x00
#define VGA_TM_CURSOR_MAX_SCANLINE 0x0F

/**
 * Initialize the VGA text mode driver.
 * 
 * @return 0 if successful, -1 if the driver failed to initialize.
 */
int32_t vga_tm_init();

/**
 * Rewrite one entry of the DAC color table.
 *
 * @param index The color index to change.
 * @param red The red component, 0 to 63.
 * @param green The green component, 0 to 63.
 * @param blue The blue component, 0 to 63.
 */
void vga_tm_set_palette_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);

/**
 * Enable or disable blinking text. While blinking is on, the high bit of the
 * background nibble makes text flash instead of selecting a bright background.
 *
 * @param enabled Whether the blink attribute is honored.
 */
void vga_tm_set_blink(bool enabled);

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
 * Scroll the VGA text mode buffer up by one line.
 * 
 * @param fgcolor The foreground color.
 * @param bgcolor The background color.
 */
void vga_tm_scroll(uint8_t fgcolor, uint8_t bgcolor);

/**
 * Get the total number of rows in the VGA text mode buffer.
 * 
 * @return The total number of rows.
 */
size_t vga_tm_total_rows();

/**
 * Get the total number of columns in the VGA text mode buffer.
 * 
 * @return The total number of columns.
 */
size_t vga_tm_total_columns();

#endif // _KERNEL_DRIVERS_VIDEO_VGA_TM_H
