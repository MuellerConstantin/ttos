#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

extern vga_gfx_screen_t vga_gfx_screen;

int32_t vga_gfx_draw_char(uint32_t x, uint32_t y, char c, uint32_t color) {
    return -1;
}
