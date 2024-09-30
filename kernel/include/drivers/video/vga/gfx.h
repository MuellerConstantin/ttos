#ifndef _KERNEL_DRIVERS_VIDEO_VGA_GFX_H
#define _KERNEL_DRIVERS_VIDEO_VGA_GFX_H

#include <drivers/video/vga/vga.h>

#define VGA_GFX_BLACK           0x00
#define VGA_GFX_BLUE            0x01
#define VGA_GFX_GREEN           0x02
#define VGA_GFX_CYAN            0x03
#define VGA_GFX_RED             0x04
#define VGA_GFX_MAGENTA         0x05
#define VGA_GFX_BROWN           0x06
#define VGA_GFX_LIGHT_GRAY      0x07
#define VGA_GFX_DARK_GRAY       0x08
#define VGA_GFX_LIGHT_BLUE      0x09
#define VGA_GFX_LIGHT_GREEN     0x0A
#define VGA_GFX_LIGHT_CYA       0x0B
#define VGA_GFX_LIGHT_RED       0x0C
#define VGA_GFX_LIGHT_MAGENTA   0x0D
#define VGA_GFX_YELLOW          0x0E
#define VGA_GFX_WHITE           0x0F

typedef struct vga_gfx_screen vga_gfx_screen_t;

struct vga_gfx_screen {
    size_t width;
    size_t height;
    size_t cursor_x;
    size_t cursor_y;
    uint8_t fgcolor;
    uint8_t bgcolor;
};

/**
 * Initialize the VGA graphics mode driver.
 * 
 * @return 0 if successful, -1 if the driver failed to initialize.
 */
int32_t vga_gfx_init();

/**
 * Clear the VGA graphics mode buffer.
 * 
 * @return 0 if successful, -1 if the buffer is not initialized.
 */
int32_t vga_gfx_clear();

/**
 * Fill the VGA graphics mode buffer with a background color.
 * 
 * @param color The background color.
 * @return 0 if successful, -1 if the buffer is not initialized.
 */
int32_t vga_gfx_fill(uint32_t color);

/**
 * Set a pixel in the VGA graphics mode buffer.
 * 
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param color The color of the pixel.
 * @return 0 if successful, -1 if the buffer is not initialized.
 */
int32_t vga_gfx_set_pixel(uint32_t x, uint32_t y, uint32_t color);

/**
 * Draw a rectangle in the VGA graphics mode buffer.
 * 
 * @param x The x-coordinate of the top-left corner of the rectangle.
 * @param y The y-coordinate of the top-left corner of the rectangle.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param color The color of the rectangle.
 * @return 0 if successful, -1 if the buffer is not initialized.
 */
int32_t vga_gfx_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);

/**
 * Draw a character in the VGA graphics mode buffer.
 * 
 * @param x The x-coordinate of the character.
 * @param y The y-coordinate of the character.
 * @param c The character to draw.
 * @param color The color of the character.
 */
int32_t vga_gfx_draw_char(uint32_t x, uint32_t y, char c, uint32_t color);

/**
 * Draw a string in the VGA graphics mode buffer.
 * 
 * @param x The x-coordinate of the string.
 * @param y The y-coordinate of the string.
 * @param str The string to draw.
 * @param color The color of the string.
 * @return 0 if successful, -1 if the buffer is not initialized.
 */
int32_t vga_gfx_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color);

#endif // _KERNEL_DRIVERS_VIDEO_VGA_GFX_H
