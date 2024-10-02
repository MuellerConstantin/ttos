#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

int32_t vga_gfx_draw_string(uint32_t x, uint32_t y, const char* str, uint32_t color) {
    uint32_t index = 0;

    while(str[index] != '\0') {
        vga_gfx_draw_char(x, y, str[index], color);
        x += 8;
        index++;
    }

    return 0;
}
