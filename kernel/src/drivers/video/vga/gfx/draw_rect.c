#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

int32_t vga_gfx_draw_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    if(x + width > vga_current_video_mode->width || y + height > vga_current_video_mode->height) {
        return -1;
    }

    for(uint32_t i = 0; i < height; ++i) {
        for(uint32_t j = 0; j < width; ++j) {
            vga_gfx_set_pixel(x + j, y + i, color);
        }
    }

    return 0;
}
