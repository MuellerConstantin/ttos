#include <drivers/video/vga/textmode.h>

extern uint16_t *const vga_tm_video_memory;

extern const vga_video_mode_descriptor_t* vga_current_video_mode;

vga_tm_screen_t vga_tm_screen;

int32_t vga_tm_init(uint8_t fgcolor, uint8_t bgcolor, bool probe) {
    vga_init(VGA_80x25_16_TEXT, probe);

    vga_tm_screen.rows = vga_current_video_mode->height;
    vga_tm_screen.columns = vga_current_video_mode->width;
    vga_tm_screen.cursor_x = 0;
    vga_tm_screen.cursor_y = 0;
    vga_tm_screen.fgcolor = fgcolor;
    vga_tm_screen.bgcolor = bgcolor;

    vga_tm_clear();
    vga_tm_enable_cursor(0, 15);
}
