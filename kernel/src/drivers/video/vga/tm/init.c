#include <drivers/video/vga/textmode.h>

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

vga_tm_screen_t vga_tm_screen;

int32_t vga_tm_init() {
    vga_tm_screen.rows = vga_current_video_mode->height;
    vga_tm_screen.columns = vga_current_video_mode->width;
    vga_tm_screen.cursor_x = 0;
    vga_tm_screen.cursor_y = 0;
    vga_tm_screen.fgcolor = VGA_TM_WHITE;
    vga_tm_screen.bgcolor = VGA_TM_BLACK;

    vga_tm_clear();
    vga_tm_enable_cursor(0, 15);
}
