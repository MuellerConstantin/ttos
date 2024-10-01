#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

extern vga_gfx_screen_t vga_gfx_screen;

int32_t vga_gfx_clear() {
    vga_gfx_fill(vga_gfx_screen.bgcolor);
}

int32_t vga_gfx_fill(uint32_t color) {
    for(size_t y = 0; y < vga_gfx_screen.height; ++y) {
        for(size_t x = 0; x < vga_gfx_screen.width; ++x) {
            vga_gfx_set_pixel(x, y, color);
        }
    }
}
