#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

extern vga_gfx_screen_t vga_gfx_screen;

int32_t vga_gfx_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if(x + width > vga_gfx_screen.width || y + height > vga_gfx_screen.height) {
        return -1;
    }

    for(uint32_t i = 0; i < height; ++i) {
        for(uint32_t j = 0; j < width; ++j) {
            vga_gfx_video_memory[(y + i) * vga_gfx_screen.width + (x + j)] = color;
        }
    }

    return 0;
}
