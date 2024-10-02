#include <drivers/video/vga/gfx.h>

extern uint8_t *const vga_gfx_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

int32_t vga_gfx_fill(uint32_t color) {
    for(size_t y = 0; y < vga_current_video_mode->height; ++y) {
        for(size_t x = 0; x < vga_current_video_mode->width; ++x) {
            vga_gfx_set_pixel(x, y, color);
        }
    }

    return 0;
}
