#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

extern vga_gfx_screen_t vga_gfx_screen;

int32_t vga_gfx_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if(x >= vga_gfx_screen.width || y >= vga_gfx_screen.height) {
        return -1;
    }

    vga_gfx_video_memory[y * vga_gfx_screen.width + x] = color;

    return 0;
}