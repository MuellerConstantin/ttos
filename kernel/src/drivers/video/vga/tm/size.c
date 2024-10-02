#include <drivers/video/vga/tm.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

size_t vga_tm_total_rows() {
    return vga_current_video_mode->height;
}

size_t vga_tm_total_columns() {
    return vga_current_video_mode->width;
}
