#include <drivers/video/vga/gfx.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

size_t vga_gfx_total_height() {
    return vga_current_video_mode->height;
}

size_t vga_gfx_total_width() {
    return vga_current_video_mode->width;
}
