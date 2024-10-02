#include <drivers/video/vga/tm.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

int32_t vga_tm_init() {
    vga_tm_fill(VGA_TM_BLACK);
    vga_tm_enable_cursor(0, 15);
}
